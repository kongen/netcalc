#include <array>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>

#include <string>

#include "netcalc_build_config.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::HasSubstr;
using ::testing::Eq;
using ::testing::Ne;

std::string shellQuote(std::string const& value)
{
	std::string quoted("'");
	for (char c : value) {
		if (c == '\'')
			quoted += "'\\''";
		else
			quoted.push_back(c);
	}
	quoted.push_back('\'');
	return quoted;
}

struct CommandResult {
	int exitCode;
	std::string output;
};

CommandResult runCommand(std::string const& command)
{
	std::array<char, 256> buffer{};
	std::string output;
	FILE* pipe = popen(command.c_str(), "r");
	EXPECT_NE(pipe, nullptr);
	if (pipe == nullptr)
		return {-1, ""};

	while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr)
		output += buffer.data();

	const int status = pclose(pipe);
	EXPECT_NE(status, -1);

	int exitCode(status);
	if (WIFEXITED(status))
		exitCode = WEXITSTATUS(status);

	return {exitCode, output};
}

std::string resolveBinaryPath(int argc, char* argv[])
{
	if (argc == 2)
		return argv[1];

	const char* topBuildDir = std::getenv("top_builddir");
	EXPECT_NE(topBuildDir, nullptr);
	if (topBuildDir == nullptr)
		return "";
	return std::string(topBuildDir) + "/src/ipv6calc";
}

std::string makeTempPath()
{
	std::string path("/tmp/ipv6calc-test-XXXXXX");
	std::vector<char> buffer(path.begin(), path.end());
	buffer.push_back('\0');
	const int fd = mkstemp(buffer.data());
	EXPECT_NE(fd, -1);
	if (fd != -1)
		close(fd);
	return std::string(buffer.data());
}

std::string readFile(std::string const& path)
{
	std::ifstream stream(path.c_str());
	EXPECT_TRUE(stream.is_open());
	return std::string((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
}

class Ipv6CalcCliTest : public ::testing::Test {
public:
	static void SetUpTestSuite()
	{
		ASSERT_FALSE(binaryPath().empty());
	}

	static void InitBinaryPath(int argc, char* argv[])
	{
		binaryPath() = resolveBinaryPath(argc, argv);
	}

	static std::string const& binary()
	{
		return binaryPath();
	}

private:
	static std::string& binaryPath()
	{
		static std::string path;
		return path;
	}
};

TEST_F(Ipv6CalcCliTest, ReportsSubnetDetailsForValidCidr)
{
	const auto result(runCommand(shellQuote(binary()) + " 2001:db8:abcd:12::1234/64 2>&1"));
	EXPECT_EQ(result.exitCode, 0);
	EXPECT_THAT(result.output, HasSubstr("Network Address: 2001:db8:abcd:12::"));
	EXPECT_THAT(result.output, HasSubstr("Last Address in Subnet: 2001:db8:abcd:12:ffff:ffff:ffff:ffff"));
	EXPECT_THAT(result.output, HasSubstr("Total number of addresses: 18446744073709551616"));
}

TEST_F(Ipv6CalcCliTest, RejectsInvalidAddress)
{
	const auto result(runCommand(shellQuote(binary()) + " not-an-ipv6-address/64 2>&1"));
	EXPECT_THAT(result.exitCode, Ne(0));
	EXPECT_THAT(result.output, HasSubstr("Error: invalid ip address"));
}

TEST_F(Ipv6CalcCliTest, RejectsOversizedPrefix)
{
	const auto result(runCommand(shellQuote(binary()) + " 2001:db8::1/129 2>&1"));
	EXPECT_THAT(result.exitCode, Ne(0));
	EXPECT_THAT(result.output, HasSubstr("Error: invalid prefix size"));
}

TEST_F(Ipv6CalcCliTest, PrintsHelp)
{
	const auto result(runCommand(shellQuote(binary()) + " --help 2>&1"));
	EXPECT_EQ(result.exitCode, 0);
	EXPECT_THAT(result.output, HasSubstr("Usage:"));
	EXPECT_THAT(result.output, HasSubstr("<ipv6 address>[/prefix size]"));
}

TEST_F(Ipv6CalcCliTest, PrintsVersion)
{
	const auto result(runCommand(shellQuote(binary()) + " --version 2>&1"));
	EXPECT_EQ(result.exitCode, 0);
	EXPECT_THAT(result.output, HasSubstr("ipv6calc "));
}

TEST_F(Ipv6CalcCliTest, PrintsToStdoutWhenExplicitlyRequested)
{
	const auto result(runCommand(shellQuote(binary()) + " " + shellQuote("--output=stdout") + " 2001:db8:abcd:12::1234/64 2>&1"));
	EXPECT_EQ(result.exitCode, 0);
	EXPECT_THAT(result.output, HasSubstr("Network Address: 2001:db8:abcd:12::"));
}

TEST_F(Ipv6CalcCliTest, PrintsToStderrWhenRequested)
{
	const std::string stdoutPath(makeTempPath());
	const std::string stderrPath(makeTempPath());
	const auto result(runCommand(shellQuote(binary()) + " --output=stderr 2001:db8:abcd:12::1234/64 1>" + shellQuote(stdoutPath) + " 2>" + shellQuote(stderrPath)));
	EXPECT_EQ(result.exitCode, 0);
	EXPECT_TRUE(result.output.empty());
	EXPECT_THAT(readFile(stdoutPath), Eq(""));
	EXPECT_THAT(readFile(stderrPath), HasSubstr("Network Address: 2001:db8:abcd:12::"));
	std::remove(stdoutPath.c_str());
	std::remove(stderrPath.c_str());
}

TEST_F(Ipv6CalcCliTest, WritesOutputToFileWhenRequested)
{
	const std::string outputPath(makeTempPath());
	const auto result(runCommand(shellQuote(binary()) + " " + shellQuote("--output=" + outputPath) + " 2001:db8:abcd:12::1234/64 2>&1"));
	EXPECT_EQ(result.exitCode, 0);
	EXPECT_TRUE(result.output.empty());
	EXPECT_THAT(readFile(outputPath), HasSubstr("Network Address: 2001:db8:abcd:12::"));
	std::remove(outputPath.c_str());
}

TEST_F(Ipv6CalcCliTest, RejectsSymbolicLinkOutputTarget)
{
	const std::string targetPath(makeTempPath());
	const std::string linkPath(targetPath + ".lnk");
	{
		std::ofstream seed(targetPath.c_str(), std::ios::out | std::ios::trunc);
		ASSERT_TRUE(seed.is_open());
		seed << "seed";
	}
	std::remove(linkPath.c_str());
	ASSERT_EQ(symlink(targetPath.c_str(), linkPath.c_str()), 0);

	const auto result(runCommand(shellQuote(binary()) + " " + shellQuote("--output=" + linkPath) + " 2001:db8:abcd:12::1234/64 2>&1"));
	EXPECT_THAT(result.exitCode, Ne(0));
	EXPECT_THAT(result.output, HasSubstr("Error: refusing unsafe output target"));
	EXPECT_THAT(readFile(targetPath), Eq("seed"));

	std::remove(linkPath.c_str());
	std::remove(targetPath.c_str());
}

#if NETCALC_HAVE_JSON
TEST_F(Ipv6CalcCliTest, PrintsJsonWhenRequested)
{
	const auto result(runCommand(shellQuote(binary()) + " --format json 2001:db8:abcd:12::1234/64 2>&1"));
	EXPECT_EQ(result.exitCode, 0);
	EXPECT_THAT(result.output, HasSubstr("\"network_address\": \"2001:db8:abcd:12::\""));
}
#endif

#if NETCALC_HAVE_XML
TEST_F(Ipv6CalcCliTest, PrintsXmlWhenRequested)
{
	const auto result(runCommand(shellQuote(binary()) + " --format xml 2001:db8:abcd:12::1234/64 2>&1"));
	EXPECT_EQ(result.exitCode, 0);
	EXPECT_THAT(result.output, HasSubstr("<network_address>2001:db8:abcd:12::</network_address>"));
}
#endif

int main(int argc, char* argv[])
{
	::testing::InitGoogleMock(&argc, argv);
	Ipv6CalcCliTest::InitBinaryPath(argc, argv);
	return RUN_ALL_TESTS();
}
