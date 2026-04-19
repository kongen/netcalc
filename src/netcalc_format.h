#ifndef NETCALC_FORMAT_H
#define NETCALC_FORMAT_H

#include <iosfwd>
#include <string>
#include <vector>

namespace netcalc {

enum class OutputFormat {
	Text,
	Json,
	Xml
};

struct OutputField {
	std::string key;
	std::string label;
	std::string value;
	bool numeric;

	OutputField(std::string const& k, std::string const& l, std::string const& v, bool isNumeric=false)
		: key(k)
		, label(l)
		, value(v)
		, numeric(isNumeric)
	{
	}
};

bool writeFormattedReport(std::ostream& out, OutputFormat format, std::string const& rootName, std::vector<OutputField> const& fields);

} // namespace netcalc

#endif
