// SPDX-License-Identifier: MIT
#include <functional>
#include <iostream>

#include "cpp_generator.hpp"
#include "utils.hpp"

static std::string_view xstype_to_cpptype(std::string_view xstype) {
	// clang-format off
	if (xstype == "xs:string")       return "std::string";
	if (xstype == "xs:boolean")      return "bool";
	if (xstype == "xs:decimal")      return "double";
	if (xstype == "xs:float")        return "float";
	if (xstype == "xs:double")       return "double";
	if (xstype == "xs:duration")     return "std::chrono::milliseconds";
	if (xstype == "xs:dateTime")     return "std::chrono::time_point<std::chrono::system_clock>";
	if (xstype == "xs:date")         return "std::chrono::year_month_day";
	if (xstype == "xs:time")         return "std::chrono::hh_mm_ss";
	if (xstype == "xs:gYearMonth")   return "std::chrono::year_month";
	if (xstype == "xs:gYear")        return "std::chrono::year";
	if (xstype == "xs:gMonthDay")    return "std::chrono::month_day";
	if (xstype == "xs:gDay")         return "std::chrono::day";
	if (xstype == "xs:gMonth")       return "std::chrono::month";
	if (xstype == "xs:base64Binary") return "std::vector<unsigned char>";
	if (xstype == "xs:anyURI")       return "std::string";
	//if (xstype == "xs:QName")        return "std::string";
	//if (xstype == "xs:NOTATION")     return "std::string";
	// clang-format on
	return "(invalid type)";
}

Generate::Generate(std::ostream& o) noexcept
	: os(o) {}

Generate& Generate::setTypeOverrides(std::map<std::string, std::string> overrides) {
	type_overrides = std::move(overrides);
}

void Generate::do_node(const rx::xml_node<>& node) const {
	const auto nodename = node_name(node);
	if (nodename == "xs:attribute") {
		auto* nameattr = get_attr(node, "name");
		auto* typeattr = get_attr(node, "type");
		if (nameattr && typeattr) {
			os << indent << attr_kv(*typeattr).second << ' ' << attr_kv(*nameattr).second << ";\n";
		}
	} else if (nodename == "xs:element") {
		auto* nameattr = get_attr(node, "name");
		if (nameattr) {
			os << indent << "struct " << attr_kv(*nameattr).second << " {\n";
			indent.push_back('\t');
			for_each(node, std::bind_front(&Generate::do_node, this));
			indent.pop_back();
			os << indent << "};\n";
		}
	} else {
		for_each(node, std::bind_front(&Generate::do_node, this));
	}
}

void Generate::operator()(const rx::xml_document<>& doc) const {
	for_each(doc, std::bind_front(&Generate::do_node, this));
}