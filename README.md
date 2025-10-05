# xsdmerge

A tool to merge XML schema definitions files.

If you have many XSD files that slightly overlap, this tool will collate all of them into one big XSD file.

## Build

Depends on Boost::program_options static lib. Should work with any recent version of Boost.

rapidxml is vendored (with a fix).

Requires C++20 or newer compiler.

## Usage

Flags:
- `i` : Path to directory to look for XSD files.
- `o` : Path to output file.
- `c` : Case insensitive attribute string comparison, default false.

## Todo

- Feature to recognize imported schemas.
- Feature to recognize and normalize attributes.

## License

MIT