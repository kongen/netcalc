#include "netcalc_format.h"

#include <cctype>
#include <iomanip>
#include <ostream>
#include <sstream>

namespace netcalc {

namespace {

std::string jsonEscape(std::string const& value)
{
	std::string escaped;
	for (std::string::const_iterator it = value.begin(); it != value.end(); ++it) {
		switch (*it) {
		case '\\':
			escaped += "\\\\";
			break;
		case '"':
			escaped += "\\\"";
			break;
		case '\b':
			escaped += "\\b";
			break;
		case '\f':
			escaped += "\\f";
			break;
		case '\n':
			escaped += "\\n";
			break;
		case '\r':
			escaped += "\\r";
			break;
		case '\t':
			escaped += "\\t";
			break;
		default:
			if (static_cast<unsigned char>(*it) < 0x20u) {
				std::ostringstream codePoint;
				codePoint << "\\u"
				          << std::hex
				          << std::nouppercase
				          << std::setw(4)
				          << std::setfill('0')
				          << static_cast<unsigned>(static_cast<unsigned char>(*it));
				escaped += codePoint.str();
			} else {
				escaped.push_back(*it);
			}
			break;
		}
	}
	return escaped;
}

std::string xmlEscape(std::string const& value)
{
	std::string escaped;
	for (std::string::const_iterator it = value.begin(); it != value.end(); ++it) {
		switch (*it) {
		case '&':
			escaped += "&amp;";
			break;
		case '<':
			escaped += "&lt;";
			break;
		case '>':
			escaped += "&gt;";
			break;
		case '\'':
			escaped += "&apos;";
			break;
		case '"':
			escaped += "&quot;";
			break;
		default:
			escaped.push_back(*it);
			break;
		}
	}
	return escaped;
}

bool isXmlNameStartChar(char c)
{
	return std::isalpha(static_cast<unsigned char>(c)) != 0 || c == '_';
}

bool isXmlNameChar(char c)
{
	return std::isalnum(static_cast<unsigned char>(c)) != 0 || c == '_' || c == '-' || c == '.';
}

std::string sanitizeXmlName(std::string const& value)
{
	if (value.empty())
		return "field";

	std::string sanitized;
	sanitized.reserve(value.size());

	for (std::string::size_type i = 0; i < value.size(); ++i) {
		const char c(value[i]);
		const bool valid = i == 0 ? isXmlNameStartChar(c) : isXmlNameChar(c);
		sanitized.push_back(valid ? c : '_');
	}

	if (!isXmlNameStartChar(sanitized[0]))
		sanitized.insert(sanitized.begin(), '_');

	return sanitized;
}

bool writeText(std::ostream& out, std::vector<OutputField> const& fields)
{
	for (std::vector<OutputField>::const_iterator it = fields.begin(); it != fields.end(); ++it) {
		if (it->value.empty())
			continue;
		out << it->label << ": " << it->value << '\n';
	}
	out.flush();
	return out.good();
}

bool writeJson(std::ostream& out, std::string const& rootName, std::vector<OutputField> const& fields)
{
	out << "{\n";
	out << "  \"" << jsonEscape(rootName) << "\": {\n";

	bool first(true);
	for (std::vector<OutputField>::const_iterator it = fields.begin(); it != fields.end(); ++it) {
		if (!first)
			out << ",\n";
		first = false;
		out << "    \"" << jsonEscape(it->key) << "\": ";
		if (it->numeric)
			out << it->value;
		else
			out << "\"" << jsonEscape(it->value) << "\"";
	}

	out << "\n  }\n";
	out << "}\n";
	out.flush();
	return out.good();
}

bool writeXml(std::ostream& out, std::string const& rootName, std::vector<OutputField> const& fields)
{
	const std::string safeRootName(sanitizeXmlName(rootName));
	out << "<" << safeRootName << ">\n";
	for (std::vector<OutputField>::const_iterator it = fields.begin(); it != fields.end(); ++it) {
		const std::string safeKey(sanitizeXmlName(it->key));
		out << "  <" << safeKey << ">" << xmlEscape(it->value) << "</" << safeKey << ">\n";
	}
	out << "</" << safeRootName << ">\n";
	out.flush();
	return out.good();
}

} // namespace

bool writeFormattedReport(std::ostream& out, OutputFormat format, std::string const& rootName, std::vector<OutputField> const& fields)
{
	switch (format) {
	case OutputFormat::Json:
		return writeJson(out, rootName, fields);
	case OutputFormat::Xml:
		return writeXml(out, rootName, fields);
	case OutputFormat::Text:
	default:
		return writeText(out, fields);
	}
}

} // namespace netcalc
