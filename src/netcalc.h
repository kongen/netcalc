#ifndef NETCALC_H
#define NETCALC_H

#include <iosfwd>
#include <memory>
#include "netcalc_format.h"
#include <sstream>
#include <string>
#include <vector>

namespace netcalc {

struct ParsedOptions {
	bool valid = true;
	bool showHelp = false;
	bool showVersion = false;
	OutputFormat format = OutputFormat::Text;
	std::string outputTarget = "stdout";
	std::vector<std::string> positionalArgs;
};

struct CalculationResult {
	bool success = false;
	std::string errorMessage;
	std::vector<OutputField> fields;
};

class Calculator {
public:
	virtual ~Calculator() = default;

	int run(int argc, char* argv[]) const;

protected:
	virtual int usage(std::ostream& out, std::string const& appName, int ret) const = 0;
	virtual std::string reportName() const = 0;
	virtual CalculationResult calculate(std::vector<std::string> const& positionalArgs, OutputFormat format) const = 0;
};

class OutputFile {
public:
	OutputFile();
	~OutputFile();

	OutputFile(OutputFile const&) = delete;
	OutputFile& operator=(OutputFile const&) = delete;

	bool open(std::string const& path, std::string& errorMessage);
	bool close();
	bool isOpen() const;
	std::ostream& stream();

private:
	struct Impl;
	std::unique_ptr<Impl> impl_;
};

std::string baseName(char const* argv0);
ParsedOptions parseOptions(int argc, char* argv[]);
bool configureOutput(std::string const& outputTarget, OutputFile& outputFile, std::ostream*& out, std::ostream& stdoutStream, std::ostream& stderrStream, std::string& errorMessage);

template<typename T>
std::string toString(T const& value)
{
	std::ostringstream stream;
	stream << value;
	return stream.str();
}

} // namespace netcalc

#endif
