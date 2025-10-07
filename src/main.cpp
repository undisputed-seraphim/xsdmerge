// SPDX-License-Identifier: MIT
#include <boost/program_options.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <rapidxml_print.hpp>
#include <vector>

#include "cpp_generator.hpp"
#include "utils.hpp"

namespace fs = std::filesystem;
namespace po = boost::program_options;
namespace rx = rapidxml;

bool case_insensitive = false;

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

	//std::ofstream(output) << outdoc;

	Generate g(std::cout);
	g(outdoc);

	return 0;
} catch (const std::exception& e) {
	std::cerr << "Exception caught in main():\n";
	std::cerr << e.what() << std::endl;
} catch (...) {
	std::cerr << "Unknown throw caught." << std::endl;
}