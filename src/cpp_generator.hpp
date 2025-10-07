// SPDX-License-Identifier: MIT
#pragma once
#include <ostream>
#include <rapidxml.hpp>
#include <string>
#include <map>

namespace rx = rapidxml;

class Generate {
	Generate(const Generate&) = delete;

	void do_node(const rx::xml_node<>&) const;

	std::ostream& os;
	std::map<std::string, std::string> type_overrides;
	mutable std::string indent;

public:
	Generate(std::ostream& o) noexcept;

	Generate& setTypeOverrides(std::map<std::string, std::string>);

	void operator()(const rx::xml_document<>&) const;
};