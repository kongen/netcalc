#include <arpa/inet.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ipv6calc.h"

using namespace ipv6calc;

using ::testing::Eq;

TEST(Ipv6PrefixParsing, AcceptsValidPrefix)
{
	EXPECT_THAT(getPrefixSize("64"), Eq(64u));
}

TEST(Ipv6PrefixParsing, RejectsOversizedPrefix)
{
	EXPECT_THAT(getPrefixSize("999999999999999999999"), Eq(129u));
}

TEST(Ipv6PrefixParsing, RejectsTrailingJunk)
{
	EXPECT_THAT(getPrefixSize("64abc"), Eq(129u));
}

TEST(Ipv6CidrParsing, DefaultsToSlash128WhenNoPrefixProvided)
{
	EXPECT_THAT(parseCidr("2001:db8::1"), Eq(std::make_pair(std::string("2001:db8::1"), 128u)));
}

TEST(Ipv6MaskConversion, ConvertsPrefixToMask)
{
	in6_addr expected{};
	ASSERT_EQ(inet_pton(AF_INET6, "ffff:ffff:ffff:ffff::", &expected), 1);
	EXPECT_THAT(to_ipv6(fromBytes(prefixSize2netmask(64))), Eq(to_ipv6(expected)));
}

TEST(Ipv6MaskConversion, RejectsInvalidPrefix)
{
	in6_addr expected{};
	EXPECT_THAT(to_ipv6(fromBytes(prefixSize2netmask(129))), Eq(to_ipv6(expected)));
}

TEST(Ipv6SubnetMath, ComputesNetworkAddress)
{
	in6_addr addr{};
	ASSERT_EQ(inet_pton(AF_INET6, "2001:db8:abcd:12::1234", &addr), 1);
	EXPECT_THAT(to_ipv6(networkAddress(addr, 64)), Eq("2001:db8:abcd:12::"));
}

TEST(Ipv6SubnetMath, RejectsInvalidPrefixForNetworkAddress)
{
	in6_addr addr{};
	in6_addr expected{};
	ASSERT_EQ(inet_pton(AF_INET6, "2001:db8:abcd:12::1234", &addr), 1);
	EXPECT_THAT(to_ipv6(networkAddress(addr, 129)), Eq(to_ipv6(expected)));
}

TEST(Ipv6SubnetMath, ComputesLastAddress)
{
	in6_addr addr{};
	ASSERT_EQ(inet_pton(AF_INET6, "2001:db8:abcd:12::1234", &addr), 1);
	EXPECT_THAT(to_ipv6(lastAddress(addr, 64)), Eq("2001:db8:abcd:12:ffff:ffff:ffff:ffff"));
}

TEST(Ipv6SubnetMath, RejectsInvalidPrefixForLastAddress)
{
	in6_addr addr{};
	in6_addr expected{};
	ASSERT_EQ(inet_pton(AF_INET6, "2001:db8:abcd:12::1234", &addr), 1);
	EXPECT_THAT(to_ipv6(lastAddress(addr, 129)), Eq(to_ipv6(expected)));
}

TEST(Ipv6SubnetMath, HandlesSlash0Boundaries)
{
	in6_addr addr{};
	ASSERT_EQ(inet_pton(AF_INET6, "2001:db8::1", &addr), 1);
	EXPECT_THAT(to_ipv6(networkAddress(addr, 0)), Eq("::"));
	EXPECT_THAT(to_ipv6(lastAddress(addr, 0)), Eq("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff"));
}

TEST(Ipv6SubnetMath, HandlesSlash128Boundaries)
{
	in6_addr addr{};
	ASSERT_EQ(inet_pton(AF_INET6, "2001:db8::1", &addr), 1);
	EXPECT_THAT(to_ipv6(networkAddress(addr, 128)), Eq("2001:db8::1"));
	EXPECT_THAT(to_ipv6(lastAddress(addr, 128)), Eq("2001:db8::1"));
}

TEST(Ipv6Counts, CalculatesSubnetSizeExactly)
{
	EXPECT_THAT(addressCount(64), Eq("18446744073709551616"));
}

TEST(Ipv6Counts, HandlesBoundaryPrefixSizes)
{
	EXPECT_THAT(addressCount(0), Eq("340282366920938463463374607431768211456"));
	EXPECT_THAT(addressCount(128), Eq("1"));
}

TEST(Ipv6Counts, RejectsInvalidPrefix)
{
	EXPECT_THAT(addressCount(129), Eq("0"));
}

TEST(Ipv6Classification, DetectsDocumentationAddress)
{
	in6_addr addr{};
	ASSERT_EQ(inet_pton(AF_INET6, "2001:db8::1", &addr), 1);
	EXPECT_THAT(getAddressType(addr), Eq("documentation"));
}

TEST(Ipv6Classification, DetectsAllSupportedAddressTypes)
{
	in6_addr unspecified{};
	EXPECT_THAT(getAddressType(unspecified), Eq("unspecified"));

	in6_addr loopback{};
	ASSERT_EQ(inet_pton(AF_INET6, "::1", &loopback), 1);
	EXPECT_THAT(getAddressType(loopback), Eq("loopback"));

	in6_addr multicast{};
	ASSERT_EQ(inet_pton(AF_INET6, "ff02::1", &multicast), 1);
	EXPECT_THAT(getAddressType(multicast), Eq("multicast"));

	in6_addr linkLocal{};
	ASSERT_EQ(inet_pton(AF_INET6, "fe80::1", &linkLocal), 1);
	EXPECT_THAT(getAddressType(linkLocal), Eq("link-local unicast"));

	in6_addr uniqueLocal{};
	ASSERT_EQ(inet_pton(AF_INET6, "fd12:3456::1", &uniqueLocal), 1);
	EXPECT_THAT(getAddressType(uniqueLocal), Eq("unique local unicast"));

	in6_addr ipv4Mapped{};
	ASSERT_EQ(inet_pton(AF_INET6, "::ffff:192.0.2.1", &ipv4Mapped), 1);
	EXPECT_THAT(getAddressType(ipv4Mapped), Eq("ipv4-mapped"));

	in6_addr globalUnicast{};
	ASSERT_EQ(inet_pton(AF_INET6, "2001:4860::8888", &globalUnicast), 1);
	EXPECT_THAT(getAddressType(globalUnicast), Eq("global unicast"));
}

TEST(Ipv6Formatting, RendersExpandedAddressInLowercaseGroups)
{
	in6_addr addr{};
	ASSERT_EQ(inet_pton(AF_INET6, "2001:db8::1", &addr), 1);
	EXPECT_THAT(to_ipv6_full(addr), Eq("2001:0db8:0000:0000:0000:0000:0000:0001"));
}
