#include <boost/program_options.hpp>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <rapidxml.hpp>
#include <rapidxml_print.hpp>
#include <vector>

namespace fs = std::filesystem;
namespace po = boost::program_options;
namespace rx = rapidxml;

bool case_insensitive = false;

static bool strequals(std::string_view l, std::string_view r) {
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

static std::string_view node_name(const rx::xml_node<>& node) { return {node.name(), node.name_size()}; }

// Get attribute as key-value pair.
static std::pair<std::string_view, std::string_view> attr_kv(const rx::xml_attribute<>& attr) {
	return {{attr.name(), attr.name_size()}, {attr.value(), attr.value_size()}};
}

// Calls a functor on each child node.
// If lambda returns true, break and return the pointer.
static rx::xml_node<>* find_if(const rx::xml_node<>& node, std::function<bool(const rx::xml_node<>&)>&& fn) {
	for (auto* childptr = node.first_node(); childptr; childptr = childptr->next_sibling()) {
		if (fn(*childptr)) {
			return childptr;
		}
	}
	return nullptr;
}
// Calls a functor on each child node.
static void for_each(const rx::xml_node<>& node, std::function<void(const rx::xml_node<>&)>&& fn) {
	for (auto* childptr = node.first_node(); childptr; childptr = childptr->next_sibling()) {
		fn(*childptr);
	}
}

// Calls a functor on each attribute.
// If lambda returns true, break and return the pointer.
static rx::xml_attribute<>* find_if(const rx::xml_node<>& node, std::function<bool(const rx::xml_attribute<>&)>&& fn) {
	for (auto* attrptr = node.first_attribute(); attrptr; attrptr = attrptr->next_attribute()) {
		if (fn(*attrptr)) {
			return attrptr;
		}
	}
	return nullptr;
}
// Calls a functor on each attribute.
static void for_each(const rx::xml_node<>& node, std::function<void(const rx::xml_attribute<>&)>&& fn) {
	for (auto* attrptr = node.first_attribute(); attrptr; attrptr = attrptr->next_attribute()) {
		fn(*attrptr);
	}
}

// Deep clone an attribute and return it.
static rx::xml_attribute<>* deep_clone(const rx::xml_attribute<>& attr, rx::memory_pool<>& alloc) {
	return alloc.allocate_attribute(
		alloc.allocate_string(attr.name(), attr.name_size()),
		alloc.allocate_string(attr.value(), attr.value_size()),
		attr.name_size(),
		attr.value_size());
}

// Deep clone a node and return it. Also clones the attributes. Not recursive.
static rx::xml_node<>* deep_clone(const rx::xml_node<>& node, rx::memory_pool<>& alloc) {
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

// Returns first attribute with matching name.
static rx::xml_attribute<>* get_attr(const rx::xml_node<>& node, std::string_view name) {
	return find_if(node, [name](const rx::xml_attribute<>& a) { return strequals(attr_kv(a).first, name); });
}

// Returns first child node with matching name.
static rx::xml_node<>* get_child(const rx::xml_node<>& node, std::string_view name) {
	return find_if(node, [name](const rx::xml_node<>& n) { return (node_name(n) == name); });
}

// Returns first child node with matching attribute. Both key and value must match.
static rx::xml_node<>* get_child(const rx::xml_node<>& node, const rx::xml_attribute<>& attr) {
	return find_if(node, [&attr](const rx::xml_node<>& n) {
		return find_if(n, [&attr](const rx::xml_attribute<>& nattr) {
			const auto [name, value] = attr_kv(attr);
			const auto [nname, nvalue] = attr_kv(nattr);
			return strequals(name, nname) && strequals(value, nvalue);
		});
	});
}

static void recurse(const rx::xml_node<>& innode, rx::xml_node<>& outnode) {
	for_each(innode, [&outnode](const rx::xml_node<>& ichild) {
		auto* ochild = [&ichild, &outnode]() -> rx::xml_node<>* {
			if (auto* attr = get_attr(ichild, "name")) {
				return get_child(outnode, *attr);
			}
			return get_child(outnode, node_name(ichild));
		}();
		if (!ochild) {
			ochild = deep_clone(ichild, *outnode.document());
			outnode.append_node(ochild);
		}
		recurse(ichild, *ochild);
	});
}

int main(int argc, char* argv[]) try {
	fs::path indir;
	fs::path output;

	po::options_description desc;
	desc.add_options()("help,h", "Print this help message")(
		"in,i", po::value<fs::path>(&indir)->required(), "Path to directory to look for XSD files.")(
		"out,o", po::value<fs::path>(&output)->required(), "Path to output file.")(
		"case,c", po::bool_switch(&case_insensitive), "Case insensitive attribute string comparison, default false.");
	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	try {
		po::notify(vm);
		if (vm.count("help")) {
			std::cout << desc << std::endl;
			return 1;
		}
	} catch (const po::required_option&) {
		std::cout << desc << '\n';
		throw;
	}

	std::cout << "Building file list...\n";
	const std::vector<fs::path> inputs = [](fs::path p) -> std::vector<fs::path> {
		if (fs::is_regular_file(p)) {
			if (p.extension() == ".xsd") {
				return {p};
			}
		}
		std::vector<fs::path> ret;
		for (const auto& entry : fs::directory_iterator(p)) {
			if (auto path = entry.path(); path.extension() == ".xsd") {
				ret.push_back(std::move(path));
			}
		}
		return ret;
	}(indir);

	std::vector<char> inbuffer;
	rx::xml_document<> indoc, outdoc;
	for (const auto& input : inputs) {
		std::cout << "Processing " << input << "... ";
		auto ifs = std::ifstream(input, std::ios::ate);
		const auto size = ifs.tellg();
		ifs.seekg(0);
		inbuffer.resize(size);
		ifs.read(inbuffer.data(), size);
		std::cout << " Read " << size << " bytes.\n";

		indoc.parse<0>(inbuffer.data());
		recurse(indoc, outdoc);
		indoc.clear();
		inbuffer.clear();
		std::cout << std::endl;
	}

	std::ofstream(output) << outdoc;

	return 0;
} catch (const std::exception& e) {
	std::cerr << "Exception caught in main():\n";
	std::cerr << e.what() << std::endl;
} catch (...) {
	std::cerr << "Unknown throw caught." << std::endl;
}