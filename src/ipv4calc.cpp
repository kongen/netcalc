/**
* Copyright © 2017 Stefan Hallas Mulvad
* ipv4 subnet calculator
*/

#include "ipv4calc.h"
#include "netcalc_build_config.h"
#include "netcalc.h"

#include <iostream>
#include <stdexcept>

namespace ipv4calc {

using namespace std;

string to_ipv4(in_addr_t v)
{
	in_addr addr({v});
	char buffer[INET_ADDRSTRLEN];
	if (inet_ntop(AF_INET, &addr, buffer, sizeof(buffer)) == nullptr)
		return "";
	return buffer;
}

unsigned netmask2prefixSize(in_addr_t netmask)
{
	netmask = ntohl(netmask);
	unsigned len(0);
	while (netmask & 0x80000000u) {
		++len;
		netmask <<= 1;
	}
	if (netmask)
		return 33;
	return len;
}

in_addr_t prefixSize2netmask(unsigned prefixSize)
{
	if (prefixSize > 32)
		return 0;
	if (prefixSize == 0)
		return 0;
	return htonl(0xffffffffu << (32 - prefixSize));
}

unsigned getPrefixSize(string const& v)
{
	size_t pos(0);
	try {
		const unsigned long prefixSize(stoul(v, &pos));
		if (pos != v.size())
			return 33;
		return static_cast<unsigned>(prefixSize);
	} catch(const std::invalid_argument&) {
		return 33;
	} catch(const std::out_of_range&) {
		return 33;
	}
}

pair<string,unsigned> parseCidr(string const& v)
{
	string::size_type pos(v.find_last_of('/'));
	if (pos == string::npos)
		return make_pair(v, 32);
	return make_pair(v.substr(0, pos), getPrefixSize(v.substr(pos+1)));
}

unsigned parseNetmask(string const& v)
{
	in_addr addr;
	if (inet_aton(v.c_str(), &addr) == 0)
		return 33;
	return netmask2prefixSize(addr.s_addr);
}

pair<string,unsigned> parseArgs(int argc, char* argv[])
{
	if (argc == 2)
		return parseCidr(argv[1]);
	if (argc == 3)
		return make_pair(argv[1], parseNetmask(argv[2]));
	return make_pair("",33);
}

pair<string, unsigned> parseArgs(vector<string> const& args)
{
	vector<char*> parsedArgv;
	parsedArgv.push_back(const_cast<char*>("ipv4calc"));
	for (vector<string>::const_iterator it = args.begin(); it != args.end(); ++it)
		parsedArgv.push_back(const_cast<char*>(it->c_str()));
	return parseArgs(static_cast<int>(parsedArgv.size()), parsedArgv.data());
}

string getNetworkClass(in_addr_t v)
{
	v = ntohl(v);
	if ((v >> 31) == 0)
		return "A";
	if ((v >> 30) == 2)
		return "B";
	if ((v >> 29) == 6)
		return "C";
	if ((v >> 28) == 14)
		return "D";
	if ((v >> 28) == 15)
		return "E";
	return "unknown";
}

int usage(ostream& out, string const& appName, int ret)
{
	out << "Usage: " << endl;
	out << "\t" << appName << " <ip address>[/prefix size]" << endl;
	out << "\t" << appName << " <ip address> <netmask>" << endl;
	out << "\t" << appName << " [--output=stdout|stderr|<file path>] <ip address>[/prefix size]" << endl;
#if NETCALC_HAVE_JSON || NETCALC_HAVE_XML
	out << "\t" << appName << " [--format <";
#if NETCALC_HAVE_JSON
	out << "json";
#if NETCALC_HAVE_XML
	out << "|";
#endif
#endif
#if NETCALC_HAVE_XML
	out << "xml";
#endif
	out << ">] [--output=stdout|stderr|<file path>] <ip address>[/prefix size]" << endl;
#endif
	out << "\t" << appName << " --help" << endl;
	out << "\t" << appName << " --version" << endl;
	out << "\tex: " << appName << " 8.8.4.4" << endl;
	out << "\tex: " << appName << " 192.168.1.1/24" << endl;
	out << "\tex: " << appName << " 172.16.54.24 255.255.252.0" << endl;
	out << "\tex: " << appName << " --output=result.txt 192.168.1.1/24" << endl;
#if NETCALC_HAVE_JSON || NETCALC_HAVE_XML
	out << "\tex: " << appName << " --format ";
#if NETCALC_HAVE_JSON
	out << "json";
#else
	out << "xml";
#endif
	out << " --output=stdout 192.168.1.1/24" << endl;
#endif
	return ret;
}

int Ipv4Calculator::usage(ostream& out, string const& appName, int ret) const
{
	return ipv4calc::usage(out, appName, ret);
}

string Ipv4Calculator::reportName() const
{
	return "ipv4calc";
}

netcalc::CalculationResult Ipv4Calculator::calculate(vector<string> const& positionalArgs, netcalc::OutputFormat format) const
{
	netcalc::CalculationResult result;

	if (positionalArgs.size() < 1 || positionalArgs.size() > 2)
		return result;

	const pair<string, unsigned> parsed(parseArgs(positionalArgs));
	const string ipAddress(parsed.first);
	const unsigned prefixSize(parsed.second);

	in_addr addr;
	if (inet_aton(ipAddress.c_str(), &addr) == 0) {
		result.errorMessage = "Error: invalid ip address";
		return result;
	}

	if (prefixSize > 32) {
		result.errorMessage = "Error: invalid " + string(positionalArgs.size() == 1 ? "prefix size" : "netmask");
		return result;
	}

	const in_addr_t netmask(prefixSize2netmask(prefixSize));
	const in_addr_t netAddr(addr.s_addr & netmask);
	const in_addr_t maxAddr(addr.s_addr | ~netmask);
	const string ipAddressText(to_ipv4(addr.s_addr));
	const string netmaskText(to_ipv4(netmask));
	const string wildcardMaskText(to_ipv4(~netmask));
	const string binaryNetmaskText(to_bits(ntohl(netmask)));
	const string cidrText(ipAddressText + "/" + netcalc::toString(prefixSize));
	const string networkText(prefixSize < 31 ? to_ipv4(netAddr) : "");
	const string broadcastText(prefixSize < 31 ? to_ipv4(maxAddr) : "");
	const unsigned long long totalHosts(prefixSize < 32 ? (1ull << (32 - prefixSize)) : 1ull);
	const in_addr_t hostOffset(prefixSize < 31 ? 1 : 0);
	const string hostRangeText(prefixSize < 32 ? to_ipv4(netAddr + htonl(hostOffset)) + " -> " + to_ipv4(maxAddr - htonl(hostOffset)) : "");
	const unsigned long long usableHosts(prefixSize < 32 ? totalHosts - (2 * hostOffset) : 0ull);
	const string classText(getNetworkClass(addr.s_addr));

	result.fields.push_back(netcalc::OutputField("ip_address", "IP Address", ipAddressText));
	result.fields.push_back(netcalc::OutputField("netmask", "Netmask", netmaskText));
	result.fields.push_back(netcalc::OutputField("wildcard_mask", "Wildcard Mask", wildcardMaskText));
	result.fields.push_back(netcalc::OutputField("binary_netmask", "Binary Netmask", binaryNetmaskText));
	result.fields.push_back(netcalc::OutputField("prefix_size", "Prefix Size", netcalc::toString(prefixSize), true));
	result.fields.push_back(netcalc::OutputField("cidr_notation", "CIDR Notation", cidrText));
	if (format == netcalc::OutputFormat::Text) {
		if (prefixSize < 31) {
			result.fields.push_back(netcalc::OutputField("network_address", "Network Address", networkText));
			result.fields.push_back(netcalc::OutputField("broadcast_address", "Broadcast Address", broadcastText));
		}
		if (prefixSize < 32) {
			result.fields.push_back(netcalc::OutputField("usable_host_range", "Usable Host Range", hostRangeText));
			result.fields.push_back(netcalc::OutputField("total_hosts", "Total number of host(s)", netcalc::toString(totalHosts), true));
			result.fields.push_back(netcalc::OutputField("usable_hosts", "Number of usable hosts", netcalc::toString(usableHosts), true));
		}
	} else {
		result.fields.push_back(netcalc::OutputField("network_address", "Network Address", networkText));
		result.fields.push_back(netcalc::OutputField("broadcast_address", "Broadcast Address", broadcastText));
		result.fields.push_back(netcalc::OutputField("usable_host_range", "Usable Host Range", hostRangeText));
		result.fields.push_back(netcalc::OutputField("total_hosts", "Total number of host(s)", netcalc::toString(prefixSize < 32 ? totalHosts : 0ull), true));
		result.fields.push_back(netcalc::OutputField("usable_hosts", "Number of usable hosts", netcalc::toString(usableHosts), true));
	}
	result.fields.push_back(netcalc::OutputField("ip_class", "IP Class", classText));
	result.success = true;
	return result;
}

} // namespace ipv4calc
