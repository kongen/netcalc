#include <cstdio>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <unistd.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "netcalc.h"
#include "netcalc_format.h"

using ::testing::Eq;
using ::testing::HasSubstr;

namespace {

class StubCalculator : public netcalc::Calculator {
protected:
	int usage(std::ostream& out, std::string const& appName, int ret) const override
	{
		out << "Usage: " << appName << '\n';
		return ret;
	}

	std::string reportName() const override
	{
		return "stub";
	}

	netcalc::CalculationResult calculate(std::vector<std::string> const& positionalArgs, netcalc::OutputFormat) const override
	{
		netcalc::CalculationResult result;
		if (positionalArgs.size() != 1) {
			result.errorMessage = "Error: missing value";
			return result;
		}
		result.fields.push_back(netcalc::OutputField("value", "Value", positionalArgs.front()));
		result.success = true;
		return result;
	}
};

} // namespace

TEST(NetcalcOptions, ParsesHelpVersionAndOutputSettings)
{
	char arg0[] = "ipv4calc";
	char arg1[] = "--help";
	char arg2[] = "--version";
	char arg3[] = "--format";
	char arg4[] = "xml";
	char arg5[] = "--output=result.txt";
	char arg6[] = "192.168.1.10/24";
	char* argv[] = {arg0, arg1, arg2, arg3, arg4, arg5, arg6};

	const netcalc::ParsedOptions options(netcalc::parseOptions(7, argv));

	EXPECT_TRUE(options.valid);
	EXPECT_TRUE(options.showHelp);
	EXPECT_TRUE(options.showVersion);
	EXPECT_THAT(options.outputTarget, Eq("result.txt"));
	EXPECT_THAT(options.positionalArgs.size(), Eq(std::size_t(1)));
	EXPECT_THAT(options.positionalArgs[0], Eq("192.168.1.10/24"));
	EXPECT_THAT(static_cast<int>(options.format), Eq(static_cast<int>(netcalc::OutputFormat::Xml)));
}

TEST(NetcalcOptions, RejectsUnknownFormat)
{
	char arg0[] = "ipv4calc";
	char arg1[] = "--format";
	char arg2[] = "yaml";
	char* argv[] = {arg0, arg1, arg2};

	EXPECT_FALSE(netcalc::parseOptions(3, argv).valid);
}

TEST(NetcalcOptions, HandlesNullArgvWhenArgcIsZero)
{
	const netcalc::ParsedOptions options(netcalc::parseOptions(0, nullptr));
	EXPECT_TRUE(options.valid);
	EXPECT_FALSE(options.showHelp);
	EXPECT_FALSE(options.showVersion);
	EXPECT_THAT(options.outputTarget, Eq("stdout"));
	EXPECT_TRUE(options.positionalArgs.empty());
}

TEST(NetcalcOptions, RejectsNullArgvWhenArgsExpected)
{
	const netcalc::ParsedOptions options(netcalc::parseOptions(1, nullptr));
	EXPECT_FALSE(options.valid);
}

TEST(NetcalcOptions, RejectsMissingOutputValue)
{
	char arg0[] = "ipv4calc";
	char arg1[] = "--output";
	char* argv[] = {arg0, arg1};

	EXPECT_FALSE(netcalc::parseOptions(2, argv).valid);
}

TEST(NetcalcOptions, RejectsEmptyOutputEqualsValue)
{
	char arg0[] = "ipv4calc";
	char arg1[] = "--output=";
	char* argv[] = {arg0, arg1};

	EXPECT_FALSE(netcalc::parseOptions(2, argv).valid);
}

TEST(NetcalcOptions, LastOutputOptionWins)
{
	char arg0[] = "ipv4calc";
	char arg1[] = "--output=first.txt";
	char arg2[] = "--output";
	char arg3[] = "second.txt";
	char arg4[] = "192.168.1.10/24";
	char* argv[] = {arg0, arg1, arg2, arg3, arg4};

	const netcalc::ParsedOptions options(netcalc::parseOptions(5, argv));
	EXPECT_TRUE(options.valid);
	EXPECT_THAT(options.outputTarget, Eq("second.txt"));
}

TEST(NetcalcOptions, RejectsNullFormatValuePointer)
{
	char arg0[] = "ipv4calc";
	char arg1[] = "--format";
	char* argv[] = {arg0, arg1, nullptr};

	EXPECT_FALSE(netcalc::parseOptions(3, argv).valid);
}

TEST(NetcalcOptions, RejectsNullOutputValuePointer)
{
	char arg0[] = "ipv4calc";
	char arg1[] = "--output";
	char* argv[] = {arg0, arg1, nullptr};

	EXPECT_FALSE(netcalc::parseOptions(3, argv).valid);
}

TEST(NetcalcOutput, UsesStdoutWhenRequested)
{
	std::ofstream outputFile;
	std::ostream* out(0);
	std::string errorMessage;

	EXPECT_TRUE(netcalc::configureOutput("stdout", outputFile, out, std::cout, std::cerr, errorMessage));
	EXPECT_THAT(out, Eq(&std::cout));
	EXPECT_TRUE(errorMessage.empty());
}

TEST(NetcalcOutput, UsesStderrWhenRequested)
{
	std::ofstream outputFile;
	std::ostream* out(0);
	std::string errorMessage;

	EXPECT_TRUE(netcalc::configureOutput("stderr", outputFile, out, std::cout, std::cerr, errorMessage));
	EXPECT_THAT(out, Eq(&std::cerr));
	EXPECT_TRUE(errorMessage.empty());
}

TEST(NetcalcOutput, WritesFormattedJsonToFile)
{
	const std::string path("/tmp/netcalc-lib-test-output.txt");
	std::ofstream outputFile;
	std::ostream* out(0);
	std::string errorMessage;

	ASSERT_TRUE(netcalc::configureOutput(path, outputFile, out, std::cout, std::cerr, errorMessage));

	std::vector<netcalc::OutputField> fields;
	fields.push_back(netcalc::OutputField("network_address", "Network Address", "192.168.1.0"));
	fields.push_back(netcalc::OutputField("prefix_size", "Prefix Size", "24", true));
	netcalc::writeFormattedReport(*out, netcalc::OutputFormat::Json, "ipv4calc", fields);
	outputFile.close();

	std::ifstream stream(path.c_str());
	ASSERT_TRUE(stream.is_open());
	const std::string contents((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

	EXPECT_THAT(contents, HasSubstr("\"network_address\": \"192.168.1.0\""));
	EXPECT_THAT(contents, HasSubstr("\"prefix_size\": 24"));

	std::remove(path.c_str());
}

TEST(NetcalcOutput, ReportsFailureForBadStream)
{
	std::ostringstream output;
	output.setstate(std::ios::badbit);

	std::vector<netcalc::OutputField> fields;
	fields.push_back(netcalc::OutputField("value", "Value", "example"));

	EXPECT_FALSE(netcalc::writeFormattedReport(output, netcalc::OutputFormat::Json, "netcalc", fields));
}

TEST(NetcalcOutput, SanitizesXmlElementNames)
{
	std::ostringstream output;
	std::vector<netcalc::OutputField> fields;
	fields.push_back(netcalc::OutputField("x></x><evil>", "Bad Key", "value"));

	ASSERT_TRUE(netcalc::writeFormattedReport(output, netcalc::OutputFormat::Xml, "bad root", fields));
	EXPECT_THAT(output.str(), HasSubstr("<bad_root>"));
	EXPECT_THAT(output.str(), HasSubstr("<x___x__evil_>value</x___x__evil_>"));
}

TEST(NetcalcOutput, EscapesJsonControlCharacters)
{
	std::ostringstream output;
	std::vector<netcalc::OutputField> fields;
	fields.push_back(netcalc::OutputField("message", "Message", "line1\nline2\t\"quoted\"\\slash"));

	ASSERT_TRUE(netcalc::writeFormattedReport(output, netcalc::OutputFormat::Json, "root", fields));
	EXPECT_THAT(output.str(), HasSubstr("\"message\": \"line1\\nline2\\t\\\"quoted\\\"\\\\slash\""));
}

TEST(NetcalcOutput, EscapesJsonLowControlCharactersAsUnicode)
{
	std::ostringstream output;
	std::string value("bad");
	value.push_back(static_cast<char>(0x01));
	value += "char";
	std::vector<netcalc::OutputField> fields;
	fields.push_back(netcalc::OutputField("message", "Message", value));

	ASSERT_TRUE(netcalc::writeFormattedReport(output, netcalc::OutputFormat::Json, "root", fields));
	EXPECT_THAT(output.str(), HasSubstr("\"message\": \"bad\\u0001char\""));
}

TEST(NetcalcOutput, EscapesXmlSpecialCharacters)
{
	std::ostringstream output;
	std::vector<netcalc::OutputField> fields;
	fields.push_back(netcalc::OutputField("message", "Message", "<tag attr=\"a&b\">'text'</tag>"));

	ASSERT_TRUE(netcalc::writeFormattedReport(output, netcalc::OutputFormat::Xml, "root", fields));
	EXPECT_THAT(output.str(), HasSubstr("&lt;tag attr=&quot;a&amp;b&quot;&gt;&apos;text&apos;&lt;/tag&gt;"));
}

TEST(NetcalcOutput, TextSkipsEmptyValuesButJsonAndXmlKeepThem)
{
	std::vector<netcalc::OutputField> fields;
	fields.push_back(netcalc::OutputField("empty", "Empty", ""));

	std::ostringstream textOutput;
	ASSERT_TRUE(netcalc::writeFormattedReport(textOutput, netcalc::OutputFormat::Text, "root", fields));
	EXPECT_THAT(textOutput.str(), Eq(""));

	std::ostringstream jsonOutput;
	ASSERT_TRUE(netcalc::writeFormattedReport(jsonOutput, netcalc::OutputFormat::Json, "root", fields));
	EXPECT_THAT(jsonOutput.str(), HasSubstr("\"empty\": \"\""));

	std::ostringstream xmlOutput;
	ASSERT_TRUE(netcalc::writeFormattedReport(xmlOutput, netcalc::OutputFormat::Xml, "root", fields));
	EXPECT_THAT(xmlOutput.str(), HasSubstr("<empty></empty>"));
}

TEST(NetcalcOutput, RefusesSymbolicLinkOutputTarget)
{
	const std::string targetPath("/tmp/netcalc-symlink-target.txt");
	const std::string linkPath("/tmp/netcalc-symlink-link.txt");
	std::remove(targetPath.c_str());
	std::remove(linkPath.c_str());
	{
		std::ofstream target(targetPath.c_str(), std::ios::out | std::ios::trunc);
		ASSERT_TRUE(target.is_open());
		target << "seed";
	}
	ASSERT_EQ(symlink(targetPath.c_str(), linkPath.c_str()), 0);

	std::ofstream outputFile;
	std::ostream* out(0);
	std::string errorMessage;
	EXPECT_FALSE(netcalc::configureOutput(linkPath, outputFile, out, std::cout, std::cerr, errorMessage));
	EXPECT_THAT(errorMessage, HasSubstr("refusing unsafe output target"));

	std::remove(linkPath.c_str());
	std::remove(targetPath.c_str());
}

TEST(NetcalcOutput, NumericFieldIsWrittenAsRawValue)
{
	std::ostringstream output;
	std::vector<netcalc::OutputField> fields;
	fields.push_back(netcalc::OutputField("prefix_size", "Prefix Size", "24", true));

	ASSERT_TRUE(netcalc::writeFormattedReport(output, netcalc::OutputFormat::Json, "root", fields));
	EXPECT_THAT(output.str(), HasSubstr("\"prefix_size\": 24"));
}

TEST(NetcalcHelpers, BaseNameHandlesCommonPathShapes)
{
	EXPECT_THAT(netcalc::baseName(nullptr), Eq(""));
	EXPECT_THAT(netcalc::baseName("/usr/local/bin/ipv4calc"), Eq("ipv4calc"));
	EXPECT_THAT(netcalc::baseName("C:\\tools\\ipv6calc.exe"), Eq("ipv6calc.exe"));
	EXPECT_THAT(netcalc::baseName("/usr/local/bin/"), Eq(""));
}

TEST(NetcalcRunner, HelpDoesNotTruncateOutputFile)
{
	const std::string path("/tmp/netcalc-help-output.txt");
	{
		std::ofstream seed(path.c_str(), std::ios::out | std::ios::trunc);
		ASSERT_TRUE(seed.is_open());
		seed << "keep me";
	}

	char arg0[] = "stubcalc";
	std::string outputArg("--output=" + path);
	char arg1[] = "--help";
	char* argv[] = {arg0, arg1, const_cast<char*>(outputArg.c_str())};

	StubCalculator calculator;
	EXPECT_EQ(calculator.run(3, argv), 0);

	std::ifstream stream(path.c_str());
	ASSERT_TRUE(stream.is_open());
	const std::string contents((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
	EXPECT_THAT(contents, Eq("keep me"));

	std::remove(path.c_str());
}

TEST(NetcalcRunner, HelpTakesPrecedenceOverInvalidOptions)
{
	char arg0[] = "stubcalc";
	char arg1[] = "--help";
	char arg2[] = "--badflag";
	char* argv[] = {arg0, arg1, arg2};

	StubCalculator calculator;
	EXPECT_EQ(calculator.run(3, argv), 0);
}

TEST(NetcalcRunner, HandlesNullArgvAndZeroArgc)
{
	StubCalculator calculator;
	EXPECT_EQ(calculator.run(0, nullptr), 1);
}
