#include "netcalc.h"
#include "netcalc_build_config.h"

#include <cerrno>
#include <fcntl.h>
#include <iostream>
#include <ostream>
#include <streambuf>
#include <unistd.h>
#include <sys/stat.h>

namespace netcalc {

namespace {

bool startsWith(std::string const& value, std::string const& prefix)
{
	return value.size() >= prefix.size() && value.compare(0, prefix.size(), prefix) == 0;
}

bool parseFormatValue(std::string const& value, OutputFormat& format)
{
	if (value == "json") {
		format = OutputFormat::Json;
		return true;
	}
	if (value == "xml") {
		format = OutputFormat::Xml;
		return true;
	}
	return false;
}

bool writeAll(int fd, char const* data, std::streamsize size)
{
	while (size > 0) {
		const ssize_t written(::write(fd, data, static_cast<size_t>(size)));
		if (written < 0) {
			if (errno == EINTR)
				continue;
			return false;
		}
		if (written == 0)
			return false;
		data += written;
		size -= written;
	}
	return true;
}

class FileDescriptorBuffer : public std::streambuf {
public:
	explicit FileDescriptorBuffer(int fd)
		: fd_(fd)
	{
	}

	~FileDescriptorBuffer() override
	{
		close();
	}

	bool isOpen() const
	{
		return fd_ >= 0;
	}

	bool close()
	{
		if (fd_ < 0)
			return true;
		const int fd(fd_);
		fd_ = -1;
		if (::close(fd) != 0) {
			if (errno == EINTR)
				return true;
			return false;
		}
		return true;
	}

protected:
	std::streamsize xsputn(char const* data, std::streamsize size) override
	{
		return writeAll(fd_, data, size) ? size : 0;
	}

	int overflow(int c) override
	{
		if (c == traits_type::eof())
			return traits_type::not_eof(c);
		const char value(static_cast<char>(c));
		return writeAll(fd_, &value, 1) ? c : traits_type::eof();
	}

private:
	int fd_;
};

int safeOpenForOutput(std::string const& path, std::string& errorMessage)
{
	int flags(O_WRONLY | O_CREAT);
#ifdef O_NOFOLLOW
	flags |= O_NOFOLLOW;
#endif
#ifdef O_CLOEXEC
	flags |= O_CLOEXEC;
#endif
#ifdef O_NONBLOCK
	flags |= O_NONBLOCK;
#endif

	int fd;
	do {
		fd = ::open(path.c_str(), flags, 0666);
	} while (fd < 0 && errno == EINTR);

	if (fd < 0) {
		const int savedErrno(errno);
		if (
#ifdef ELOOP
			savedErrno == ELOOP ||
#endif
			savedErrno == EISDIR
#ifdef ENXIO
			|| savedErrno == ENXIO
#endif
		) {
			errorMessage = "Error: refusing unsafe output target '" + path + "'";
		} else {
			errorMessage = "Error: unable to open output file '" + path + "'";
		}
		return -1;
	}

	struct stat info;
	if (::fstat(fd, &info) != 0 || !S_ISREG(info.st_mode)) {
		::close(fd);
		errorMessage = "Error: refusing unsafe output target '" + path + "'";
		return -1;
	}

	int truncateStatus;
	do {
		truncateStatus = ::ftruncate(fd, 0);
	} while (truncateStatus != 0 && errno == EINTR);

	if (truncateStatus != 0 || ::lseek(fd, 0, SEEK_SET) < 0) {
		::close(fd);
		errorMessage = "Error: unable to open output file '" + path + "'";
		return -1;
	}

	return fd;
}

} // namespace

struct OutputFile::Impl {
	explicit Impl(int fd)
		: buffer(fd)
		, stream(&buffer)
	{
	}

	FileDescriptorBuffer buffer;
	std::ostream stream;
};

OutputFile::OutputFile()
	: impl_()
{
}

OutputFile::~OutputFile() = default;

bool OutputFile::open(std::string const& path, std::string& errorMessage)
{
	if (impl_)
		close();

	const int fd(safeOpenForOutput(path, errorMessage));
	if (fd < 0)
		return false;

	impl_.reset(new Impl(fd));
	return true;
}

bool OutputFile::close()
{
	if (!impl_)
		return true;
	impl_->stream.flush();
	const bool streamOk(impl_->stream.good());
	const bool closeOk(impl_->buffer.close());
	impl_.reset();
	return streamOk && closeOk;
}

bool OutputFile::isOpen() const
{
	return impl_ && impl_->buffer.isOpen();
}

std::ostream& OutputFile::stream()
{
	return impl_->stream;
}

int Calculator::run(int argc, char* argv[]) const
{
	const char* argv0 = (argv != 0 && argc > 0) ? argv[0] : "";
	const std::string appName(baseName(argv0));
	const ParsedOptions options(parseOptions(argc, argv));
	OutputFile outputFile;
	std::ostream* out(&std::cout);
	std::string errorMessage;

	if (options.showHelp)
		return usage(std::cout, appName, 0);
	if (options.showVersion) {
		std::cout << appName << " " << NETCALC_VERSION << std::endl;
		return 0;
	}
	if (!options.valid)
		return usage(std::cerr, appName, 1);

	const CalculationResult result(calculate(options.positionalArgs, options.format));
	if (!result.success) {
		if (!result.errorMessage.empty())
			std::cerr << result.errorMessage << std::endl;
		return usage(std::cerr, appName, 1);
	}

	if (!configureOutput(options.outputTarget, outputFile, out, std::cout, std::cerr, errorMessage)) {
		std::cerr << errorMessage << std::endl;
		return 1;
	}

	if (!writeFormattedReport(*out, options.format, reportName(), result.fields)) {
		std::cerr << "Error: failed to write output" << std::endl;
		return 1;
	}

	if (outputFile.isOpen()) {
		if (!outputFile.close()) {
			std::cerr << "Error: failed to write output" << std::endl;
			return 1;
		}
	}

	return 0;
}

std::string baseName(char const* argv0)
{
	if (argv0 == 0)
		return "";
	std::string path(argv0);
	const std::string::size_type pos(path.find_last_of("/\\"));
	if (pos == std::string::npos)
		return path;
	return path.substr(pos + 1);
}

ParsedOptions parseOptions(int argc, char* argv[])
{
	ParsedOptions options;
	if (argc <= 0)
		return options;
	if (argv == 0) {
		options.valid = false;
		return options;
	}

	for (int i = 1; i < argc; ++i) {
		if (argv[i] == 0) {
			options.valid = false;
			continue;
		}
		const std::string arg(argv[i]);

		if (arg == "--help") {
			options.showHelp = true;
			continue;
		}
		if (arg == "--version") {
			options.showVersion = true;
			continue;
		}
		if (startsWith(arg, "--format=")) {
			if (!parseFormatValue(arg.substr(9), options.format))
				options.valid = false;
			continue;
		}
		if (arg == "--format") {
			if (i + 1 >= argc) {
				options.valid = false;
				continue;
			}
			++i;
			if (argv[i] == 0 || !parseFormatValue(argv[i], options.format))
				options.valid = false;
			continue;
		}
		if (startsWith(arg, "--output=")) {
			options.outputTarget = arg.substr(9);
			if (options.outputTarget.empty())
				options.valid = false;
			continue;
		}
		if (arg == "--output") {
			if (i + 1 >= argc) {
				options.valid = false;
				continue;
			}
			++i;
			if (argv[i] == 0) {
				options.valid = false;
				continue;
			}
			options.outputTarget = argv[i];
			if (options.outputTarget.empty())
				options.valid = false;
			continue;
		}
		if (!arg.empty() && arg[0] == '-') {
			options.valid = false;
			continue;
		}
		options.positionalArgs.push_back(arg);
	}

	return options;
}

bool configureOutput(std::string const& outputTarget, OutputFile& outputFile, std::ostream*& out, std::ostream& stdoutStream, std::ostream& stderrStream, std::string& errorMessage)
{
	if (outputTarget.empty() || outputTarget == "stdout") {
		out = &stdoutStream;
		return true;
	}
	if (outputTarget == "stderr") {
		out = &stderrStream;
		return true;
	}

	if (!outputFile.open(outputTarget, errorMessage)) {
		out = &stdoutStream;
		return false;
	}

	out = &outputFile.stream();
	return true;
}
} // namespace netcalc
