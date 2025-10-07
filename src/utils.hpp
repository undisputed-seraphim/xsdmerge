// SPDX-License-Identifier: MIT
#pragma once

#include <functional>
#include <rapidxml.hpp>
#include <string_view>

namespace rx = rapidxml;

bool strequals(std::string_view l, std::string_view r);

std::string_view node_name(const rx::xml_node<>& node);

// Get attribute as key-value pair.
std::pair<std::string_view, std::string_view> attr_kv(const rx::xml_attribute<>& attr);

// Calls a functor on each child node.
// If lambda returns true, break and return the pointer.
rx::xml_node<>* find_if(const rx::xml_node<>& node, std::function<bool(const rx::xml_node<>&)>&& fn);
// Calls a functor on each child node.
void for_each(const rx::xml_node<>& node, std::function<void(const rx::xml_node<>&)>&& fn);

// Calls a functor on each attribute.
// If lambda returns true, break and return the pointer.
rx::xml_attribute<>* find_if(const rx::xml_node<>& node, std::function<bool(const rx::xml_attribute<>&)>&& fn);
// Calls a functor on each attribute.
void for_each(const rx::xml_node<>& node, std::function<void(const rx::xml_attribute<>&)>&& fn);

// Deep clone an attribute and return it.
rx::xml_attribute<>* deep_clone(const rx::xml_attribute<>& attr, rx::memory_pool<>& alloc);

// Deep clone a node and return it. Also clones the attributes. Not recursive.
rx::xml_node<>* deep_clone(const rx::xml_node<>& node, rx::memory_pool<>& alloc);

// Returns first attribute with matching name.
rx::xml_attribute<>* get_attr(const rx::xml_node<>& node, std::string_view name);

// Returns first child node with matching name.
rx::xml_node<>* get_child(const rx::xml_node<>& node, std::string_view name);

// Returns first child node with matching attribute. Both key and value must match.
rx::xml_node<>* get_child(const rx::xml_node<>& node, const rx::xml_attribute<>& attr);
