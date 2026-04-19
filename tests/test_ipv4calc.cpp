#include <arpa/inet.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ipv4calc.h"

using namespace ipv4calc;

using ::testing::Eq;

TEST(PrefixParsing, AcceptsValidPrefix)
{
	EXPECT_THAT(getPrefixSize("24"), Eq(24u));
}

TEST(PrefixParsing, RejectsOversizedPrefix)
{
	EXPECT_THAT(getPrefixSize("999999999999999999999"), Eq(33u));
}

TEST(PrefixParsing, RejectsTrailingJunk)
{
	EXPECT_THAT(getPrefixSize("24abc"), Eq(33u));
}

TEST(NetmaskParsing, AcceptsValidNetmask)
{
	EXPECT_THAT(parseNetmask("255.255.255.0"), Eq(24u));
}

TEST(NetmaskParsing, RejectsInvalidNetmaskString)
{
	EXPECT_THAT(parseNetmask("foo"), Eq(33u));
}

TEST(NetmaskParsing, RejectsOutOfRangeNetmaskOctet)
{
	EXPECT_THAT(parseNetmask("300.1.1.1"), Eq(33u));
}

TEST(NetmaskParsing, RejectsNonContiguousNetmask)
{
	EXPECT_THAT(parseNetmask("255.0.255.0"), Eq(33u));
}

TEST(NetmaskConversion, ConvertsPrefixToNetmask)
{
	EXPECT_THAT(prefixSize2netmask(24), Eq(inet_addr("255.255.255.0")));
}

TEST(NetmaskConversion, RejectsInvalidPrefixWithoutUndefinedBehavior)
{
	EXPECT_THAT(prefixSize2netmask(33), Eq(in_addr_t{0}));
}

TEST(NetmaskConversion, ConvertsNetmaskToPrefix)
{
	EXPECT_THAT(netmask2prefixSize(inet_addr("255.255.252.0")), Eq(22u));
}

TEST(CidrParsing, DefaultsToSlash32WhenNoPrefixProvided)
{
	EXPECT_THAT(parseCidr("10.0.0.1"), Eq(std::make_pair(std::string("10.0.0.1"), 32u)));
}

TEST(CidrParsing, RejectsEmptyPrefix)
{
	EXPECT_THAT(parseCidr("10.0.0.1/"), Eq(std::make_pair(std::string("10.0.0.1"), 33u)));
}

TEST(NetmaskConversion, ConvertsZeroPrefixToZeroMask)
{
	EXPECT_THAT(prefixSize2netmask(0), Eq(in_addr_t{0}));
}

TEST(NetmaskConversion, ConvertsSlash32PrefixToFullMask)
{
	EXPECT_THAT(prefixSize2netmask(32), Eq(inet_addr("255.255.255.255")));
}

TEST(NetmaskConversion, ConvertsZeroMaskToZeroPrefix)
{
	EXPECT_THAT(netmask2prefixSize(inet_addr("0.0.0.0")), Eq(0u));
}

TEST(NetworkClass, DetectsMulticastClass)
{
	EXPECT_THAT(getNetworkClass(inet_addr("224.0.0.1")), Eq("D"));
}

TEST(NetworkClass, DetectsRemainingClasses)
{
	EXPECT_THAT(getNetworkClass(inet_addr("8.8.8.8")), Eq("A"));
	EXPECT_THAT(getNetworkClass(inet_addr("172.16.0.1")), Eq("B"));
	EXPECT_THAT(getNetworkClass(inet_addr("192.168.1.1")), Eq("C"));
	EXPECT_THAT(getNetworkClass(inet_addr("240.0.0.1")), Eq("E"));
}
