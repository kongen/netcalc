#ifndef IPV6CALC_H
#define IPV6CALC_H

#include <array>
#include <iosfwd>
#include <string>
#include <utility>
#include <vector>

#include <arpa/inet.h>

#include "netcalc.h"

namespace ipv6calc {

using Bytes = std::array<unsigned char, 16>;

std::string to_ipv6(in6_addr const& v);
std::string to_ipv6_full(in6_addr const& v);
unsigned getPrefixSize(std::string const& v);
std::pair<std::string, unsigned> parseCidr(std::string const& v);
std::pair<std::string, unsigned> parseArgs(int argc, char* argv[]);
std::pair<std::string, unsigned> parseArgs(std::vector<std::string> const& args);
int usage(std::ostream& out, std::string const& appName, int ret=0);

Bytes toBytes(in6_addr const& addr);
in6_addr fromBytes(Bytes const& bytes);
Bytes prefixSize2netmask(unsigned prefixSize);
in6_addr networkAddress(in6_addr const& addr, unsigned prefixSize);
in6_addr lastAddress(in6_addr const& addr, unsigned prefixSize);
std::string addressCount(unsigned prefixSize);
std::string getAddressType(in6_addr const& addr);

class Ipv6Calculator : public netcalc::Calculator {
protected:
	int usage(std::ostream& out, std::string const& appName, int ret) const override;
	std::string reportName() const override;
	netcalc::CalculationResult calculate(std::vector<std::string> const& positionalArgs, netcalc::OutputFormat format) const override;
};

} // namespace ipv6calc

#endif
