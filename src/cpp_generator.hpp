// SPDX-License-Identifier: MIT
#pragma once
#include <ostream>
#include <rapidxml.hpp>
#include <string>

namespace rx = rapidxml;

class Generate {
	Generate(const Generate&) = delete;

	void do_node(const rx::xml_node<>&) const;

	std::ostream& os;
    mutable std::string indent;

public:
	Generate(std::ostream& o) noexcept;

	void operator()(const rx::xml_document<>&) const;
};