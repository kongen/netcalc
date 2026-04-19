#ifndef IPV4CALC_H
#define IPV4CALC_H

#include <iosfwd>
#include <string>
#include <utility>
#include <vector>

#include <arpa/inet.h>

#include "netcalc.h"

namespace ipv4calc {

template<typename T> std::string to_bits(T v)
{
	std::string s;
	unsigned n(sizeof(T) * 8);
	while (n-- > 0)
		s.push_back((v & (T(1) << n)) ? '1' : '0');
	return s;
}

std::string to_ipv4(in_addr_t v);
unsigned netmask2prefixSize(in_addr_t netmask);
in_addr_t prefixSize2netmask(unsigned prefixSize);
unsigned getPrefixSize(std::string const& v);
std::pair<std::string, unsigned> parseCidr(std::string const& v);
unsigned parseNetmask(std::string const& v);
std::pair<std::string, unsigned> parseArgs(int argc, char* argv[]);
std::pair<std::string, unsigned> parseArgs(std::vector<std::string> const& args);
std::string getNetworkClass(in_addr_t v);
int usage(std::ostream& out, std::string const& appName, int ret=0);

class Ipv4Calculator : public netcalc::Calculator {
protected:
	int usage(std::ostream& out, std::string const& appName, int ret) const override;
	std::string reportName() const override;
	netcalc::CalculationResult calculate(std::vector<std::string> const& positionalArgs, netcalc::OutputFormat format) const override;
};

} // namespace ipv4calc

#endif
