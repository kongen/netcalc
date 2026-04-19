#include "netcalc.h"
#include "netcalc_build_config.h"

#include <iostream>
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

bool isUnsafeOutputTarget(std::string const& outputTarget)
{
	struct stat info;
	if (lstat(outputTarget.c_str(), &info) != 0)
		return false;
	if (S_ISLNK(info.st_mode))
		return true;
	if (!S_ISREG(info.st_mode))
		return true;
	return false;
}

} // namespace

int Calculator::run(int argc, char* argv[]) const
{
	const char* argv0 = (argv != 0 && argc > 0) ? argv[0] : "";
	const std::string appName(baseName(argv0));
	const ParsedOptions options(parseOptions(argc, argv));
	std::ofstream outputFile;
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

	if (outputFile.is_open()) {
		outputFile.close();
		if (!outputFile.good()) {
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

bool configureOutput(std::string const& outputTarget, std::ofstream& outputFile, std::ostream*& out, std::ostream& stdoutStream, std::ostream& stderrStream, std::string& errorMessage)
{
	if (outputTarget.empty() || outputTarget == "stdout") {
		out = &stdoutStream;
		return true;
	}
	if (outputTarget == "stderr") {
		out = &stderrStream;
		return true;
	}
	if (isUnsafeOutputTarget(outputTarget)) {
		errorMessage = "Error: refusing unsafe output target '" + outputTarget + "'";
		out = &stdoutStream;
		return false;
	}

	outputFile.open(outputTarget.c_str(), std::ios::out | std::ios::trunc);
	if (!outputFile.is_open()) {
		errorMessage = "Error: unable to open output file '" + outputTarget + "'";
		out = &stdoutStream;
		return false;
	}

	out = &outputFile;
	return true;
}
} // namespace netcalc
