// SPDX-License-Identifier: MIT
#include <functional>
#include <iostream>

#include "cpp_generator.hpp"
#include "utils.hpp"

static std::string_view xstype_to_cpptype(std::string_view xstype) {
	// clang-format off
    if (xstype == "xs:string") return "std::string";
	// clang-format on
	return "(invalid type)";
}

Generate::Generate(std::ostream& o) noexcept
	: os(o) {}

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