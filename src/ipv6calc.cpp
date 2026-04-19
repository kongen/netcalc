/**
* Copyright © 2017 Stefan Hallas Mulvad
* ipv6 subnet calculator
*/

#include "ipv6calc.h"
#include "netcalc_build_config.h"
#include "netcalc.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace ipv6calc {

using namespace std;

string to_ipv6(in6_addr const& v)
{
	char buffer[INET6_ADDRSTRLEN];
	if (inet_ntop(AF_INET6, &v, buffer, sizeof(buffer)) == nullptr)
		return "";
	return buffer;
}

string to_ipv6_full(in6_addr const& v)
{
	const Bytes bytes(toBytes(v));
	ostringstream stream;
	stream << hex << nouppercase << setfill('0');
	for (size_t i = 0; i < bytes.size(); i += 2) {
		if (i != 0)
			stream << ":";
		stream << setw(2) << static_cast<unsigned>(bytes[i])
		       << setw(2) << static_cast<unsigned>(bytes[i + 1]);
	}
	return stream.str();
}

unsigned getPrefixSize(string const& v)
{
	size_t pos(0);
	try {
		const unsigned long prefixSize(stoul(v, &pos));
		if (pos != v.size())
			return 129;
		return static_cast<unsigned>(prefixSize);
	} catch(const invalid_argument&) {
		return 129;
	} catch(const out_of_range&) {
		return 129;
	}
}

pair<string, unsigned> parseCidr(string const& v)
{
	const string::size_type pos(v.find_last_of('/'));
	if (pos == string::npos)
		return make_pair(v, 128);
	return make_pair(v.substr(0, pos), getPrefixSize(v.substr(pos + 1)));
}

pair<string, unsigned> parseArgs(int argc, char* argv[])
{
	if (argc == 2)
		return parseCidr(argv[1]);
	if (argc == 3)
		return make_pair(argv[1], getPrefixSize(argv[2]));
	return make_pair("", 129);
}

pair<string, unsigned> parseArgs(vector<string> const& args)
{
	vector<char*> parsedArgv;
	parsedArgv.push_back(const_cast<char*>("ipv6calc"));
	for (vector<string>::const_iterator it = args.begin(); it != args.end(); ++it)
		parsedArgv.push_back(const_cast<char*>(it->c_str()));
	return parseArgs(static_cast<int>(parsedArgv.size()), parsedArgv.data());
}

int usage(ostream& out, string const& appName, int ret)
{
	out << "Usage: " << endl;
	out << "\t" << appName << " <ipv6 address>[/prefix size]" << endl;
	out << "\t" << appName << " <ipv6 address> <prefix size>" << endl;
	out << "\t" << appName << " [--output=stdout|stderr|<file path>] <ipv6 address>[/prefix size]" << endl;
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
	out << ">] [--output=stdout|stderr|<file path>] <ipv6 address>[/prefix size]" << endl;
#endif
	out << "\t" << appName << " --help" << endl;
	out << "\t" << appName << " --version" << endl;
	out << "\tex: " << appName << " 2001:db8::1" << endl;
	out << "\tex: " << appName << " 2001:db8:abcd::1/64" << endl;
	out << "\tex: " << appName << " fd12:3456:789a::42 56" << endl;
	out << "\tex: " << appName << " --output=result.txt 2001:db8::1/64" << endl;
#if NETCALC_HAVE_JSON || NETCALC_HAVE_XML
	out << "\tex: " << appName << " --format ";
#if NETCALC_HAVE_JSON
	out << "json";
#else
	out << "xml";
#endif
	out << " --output=stdout 2001:db8::1/64" << endl;
#endif
	return ret;
}

int Ipv6Calculator::usage(ostream& out, string const& appName, int ret) const
{
	return ipv6calc::usage(out, appName, ret);
}

string Ipv6Calculator::reportName() const
{
	return "ipv6calc";
}

netcalc::CalculationResult Ipv6Calculator::calculate(vector<string> const& positionalArgs, netcalc::OutputFormat format) const
{
	(void)format;
	netcalc::CalculationResult result;

	if (positionalArgs.size() < 1 || positionalArgs.size() > 2)
		return result;

	const pair<string, unsigned> parsed(parseArgs(positionalArgs));
	const string ipAddress(parsed.first);
	const unsigned prefixSize(parsed.second);

	in6_addr addr{};
	if (inet_pton(AF_INET6, ipAddress.c_str(), &addr) != 1) {
		result.errorMessage = "Error: invalid ip address";
		return result;
	}

	if (prefixSize > 128) {
		result.errorMessage = "Error: invalid prefix size";
		return result;
	}

	const Bytes netmask(prefixSize2netmask(prefixSize));
	const in6_addr network(networkAddress(addr, prefixSize));
	const in6_addr last(lastAddress(addr, prefixSize));
	const in6_addr mask(fromBytes(netmask));
	const string ipAddressText(to_ipv6(addr));
	const string expandedAddressText(to_ipv6_full(addr));
	const string prefixMaskText(to_ipv6(mask));
	const string expandedMaskText(to_ipv6_full(mask));
	const string cidrText(ipAddressText + "/" + netcalc::toString(prefixSize));
	const string networkText(to_ipv6(network));
	const string lastText(to_ipv6(last));
	const string totalAddressesText(addressCount(prefixSize));
	const string addressTypeText(getAddressType(addr));

	result.fields.push_back(netcalc::OutputField("ip_address", "IP Address", ipAddressText));
	result.fields.push_back(netcalc::OutputField("expanded_address", "Expanded Address", expandedAddressText));
	result.fields.push_back(netcalc::OutputField("prefix_mask", "Prefix Mask", prefixMaskText));
	result.fields.push_back(netcalc::OutputField("expanded_prefix_mask", "Expanded Prefix Mask", expandedMaskText));
	result.fields.push_back(netcalc::OutputField("prefix_size", "Prefix Size", netcalc::toString(prefixSize), true));
	result.fields.push_back(netcalc::OutputField("cidr_notation", "CIDR Notation", cidrText));
	result.fields.push_back(netcalc::OutputField("network_address", "Network Address", networkText));
	result.fields.push_back(netcalc::OutputField("first_address_in_subnet", "First Address in Subnet", networkText));
	result.fields.push_back(netcalc::OutputField("last_address_in_subnet", "Last Address in Subnet", lastText));
	result.fields.push_back(netcalc::OutputField("total_addresses", "Total number of addresses", totalAddressesText));
	result.fields.push_back(netcalc::OutputField("address_type", "Address Type", addressTypeText));
	result.success = true;
	return result;
}

Bytes toBytes(in6_addr const& addr)
{
	Bytes bytes{};
	for (size_t i = 0; i < bytes.size(); ++i)
		bytes[i] = addr.s6_addr[i];
	return bytes;
}

in6_addr fromBytes(Bytes const& bytes)
{
	in6_addr addr{};
	for (size_t i = 0; i < bytes.size(); ++i)
		addr.s6_addr[i] = bytes[i];
	return addr;
}

Bytes prefixSize2netmask(unsigned prefixSize)
{
	Bytes mask{};
	if (prefixSize > 128)
		return mask;
	unsigned remaining(prefixSize);
	for (auto& byte : mask) {
		if (remaining >= 8) {
			byte = 0xffu;
			remaining -= 8;
		} else if (remaining > 0) {
			byte = static_cast<unsigned char>(0xffu << (8 - remaining));
			remaining = 0;
		} else {
			byte = 0x00u;
		}
	}
	return mask;
}

in6_addr networkAddress(in6_addr const& addr, unsigned prefixSize)
{
	if (prefixSize > 128)
		return in6_addr{};
	const Bytes bytes(toBytes(addr));
	const Bytes mask(prefixSize2netmask(prefixSize));
	Bytes network{};
	for (size_t i = 0; i < bytes.size(); ++i)
		network[i] = static_cast<unsigned char>(bytes[i] & mask[i]);
	return fromBytes(network);
}

in6_addr lastAddress(in6_addr const& addr, unsigned prefixSize)
{
	if (prefixSize > 128)
		return in6_addr{};
	const Bytes bytes(toBytes(addr));
	const Bytes mask(prefixSize2netmask(prefixSize));
	Bytes last{};
	for (size_t i = 0; i < bytes.size(); ++i)
		last[i] = static_cast<unsigned char>((bytes[i] & mask[i]) | static_cast<unsigned char>(~mask[i]));
	return fromBytes(last);
}

string addressCount(unsigned prefixSize)
{
	if (prefixSize > 128)
		return "0";
	unsigned hostBits(128 - prefixSize);
	string value("1");
	for (unsigned i = 0; i < hostBits; ++i) {
		int carry(0);
		for (auto it = value.rbegin(); it != value.rend(); ++it) {
			const int digit((*it - '0') * 2 + carry);
			*it = static_cast<char>('0' + (digit % 10));
			carry = digit / 10;
		}
		if (carry != 0)
			value.insert(value.begin(), static_cast<char>('0' + carry));
	}
	return value;
}

string getAddressType(in6_addr const& addr)
{
	const Bytes bytes(toBytes(addr));

	bool allZero(true);
	for (unsigned char byte : bytes) {
		if (byte != 0) {
			allZero = false;
			break;
		}
	}
	if (allZero)
		return "unspecified";

	Bytes loopback{};
	loopback[15] = 1;
	if (bytes == loopback)
		return "loopback";

	if (bytes[0] == 0xffu)
		return "multicast";
	if (bytes[0] == 0xfeu && (bytes[1] & 0xc0u) == 0x80u)
		return "link-local unicast";
	if ((bytes[0] & 0xfeu) == 0xfcu)
		return "unique local unicast";
	if (bytes[0] == 0x20u && bytes[1] == 0x01u && bytes[2] == 0x0du && bytes[3] == 0xb8u)
		return "documentation";

	bool mapped(true);
	for (size_t i = 0; i < 10; ++i) {
		if (bytes[i] != 0) {
			mapped = false;
			break;
		}
	}
	if (mapped && bytes[10] == 0xffu && bytes[11] == 0xffu)
		return "ipv4-mapped";

	return "global unicast";
}

} // namespace ipv6calc
