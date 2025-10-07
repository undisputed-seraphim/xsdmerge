// SPDX-License-Identifier: MIT
#include "utils.hpp"
#include <cstring>

extern bool case_insensitive;

bool strequals(std::string_view l, std::string_view r) {
	if (!case_insensitive) {
		return (l == r);
	}
	if (l.size() != r.size()) {
		return false;
	}
#ifdef _WIN32
	return 0 == ::strnicmp(l.data(), r.data(), l.size());
#else
	return 0 == ::strncasecmp(l.data(), r.data(), l.size());
#endif
}

std::string_view node_name(const rx::xml_node<>& node) { return {node.name(), node.name_size()}; }

std::pair<std::string_view, std::string_view> attr_kv(const rx::xml_attribute<>& attr) {
	return {{attr.name(), attr.name_size()}, {attr.value(), attr.value_size()}};
}

rx::xml_node<>* find_if(const rx::xml_node<>& node, std::function<bool(const rx::xml_node<>&)>&& fn) {
	for (auto* childptr = node.first_node(); childptr; childptr = childptr->next_sibling()) {
		if (fn(*childptr)) {
			return childptr;
		}
	}
	return nullptr;
}

void for_each(const rx::xml_node<>& node, std::function<void(const rx::xml_node<>&)>&& fn) {
	for (auto* childptr = node.first_node(); childptr; childptr = childptr->next_sibling()) {
		fn(*childptr);
	}
}

rx::xml_attribute<>* find_if(const rx::xml_node<>& node, std::function<bool(const rx::xml_attribute<>&)>&& fn) {
	for (auto* attrptr = node.first_attribute(); attrptr; attrptr = attrptr->next_attribute()) {
		if (fn(*attrptr)) {
			return attrptr;
		}
	}
	return nullptr;
}

void for_each(const rx::xml_node<>& node, std::function<void(const rx::xml_attribute<>&)>&& fn) {
	for (auto* attrptr = node.first_attribute(); attrptr; attrptr = attrptr->next_attribute()) {
		fn(*attrptr);
	}
}

rx::xml_attribute<>* deep_clone(const rx::xml_attribute<>& attr, rx::memory_pool<>& alloc) {
	return alloc.allocate_attribute(
		alloc.allocate_string(attr.name(), attr.name_size()),
		alloc.allocate_string(attr.value(), attr.value_size()),
		attr.name_size(),
		attr.value_size());
}

rx::xml_node<>* deep_clone(const rx::xml_node<>& node, rx::memory_pool<>& alloc) {
	auto* newnode = alloc.allocate_node(
		node.type(),
		alloc.allocate_string(node.name(), node.name_size()),
		alloc.allocate_string(node.value(), node.value_size()),
		node.name_size(),
		node.value_size());
	for_each(node, [&alloc, &newnode](const rx::xml_attribute<>& attr) {
		auto* newattr = deep_clone(attr, alloc);
		newnode->append_attribute(newattr);
	});
	return newnode;
}

rx::xml_attribute<>* get_attr(const rx::xml_node<>& node, std::string_view name) {
	return find_if(node, [name](const rx::xml_attribute<>& a) { return strequals(attr_kv(a).first, name); });
}

rx::xml_node<>* get_child(const rx::xml_node<>& node, std::string_view name) {
	return find_if(node, [name](const rx::xml_node<>& n) { return (node_name(n) == name); });
}

rx::xml_node<>* get_child(const rx::xml_node<>& node, const rx::xml_attribute<>& attr) {
	return find_if(node, [&attr](const rx::xml_node<>& n) {
		return find_if(n, [&attr](const rx::xml_attribute<>& nattr) {
			const auto [name, value] = attr_kv(attr);
			const auto [nname, nvalue] = attr_kv(nattr);
			return strequals(name, nname) && strequals(value, nvalue);
		});
	});
}