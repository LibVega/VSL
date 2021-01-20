/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

/// The main function entry point for the command-line VSL compiler 'vslc'

#include "../vsl/Shader.hpp"

#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;


// ====================================================================================================================
// Return is [isFlag, argName, paramName, paramValue]
static std::tuple<bool, std::string, std::string, std::string> normalizeArg(const std::string& arg)
{
	if (arg[0] == '-') {
		const auto isLong = (arg.length() > 1) && (arg[1] == '-');
		const auto valIndex = arg.find('=');
		const auto hasValue = valIndex != std::string::npos;
		if (arg.length() == (isLong ? 2 : 1)) {
			return std::make_tuple(false, "", "", "");
		}

		if (hasValue) {
			return isLong
				? std::make_tuple(true, arg.substr(2, valIndex - 2), "", arg.substr(valIndex + 1))
				: std::make_tuple(true, std::string{ arg[1] }, arg.substr(2, valIndex - 2), arg.substr(valIndex + 1));
		}
		else {
			return isLong
				? std::make_tuple(true, arg.substr(2), "", "")
				: std::make_tuple(true, std::string{ arg[1] }, arg.substr(2), "");
		}
	}
	else {
		return std::make_tuple(false, arg, "", "");
	}
}

// ====================================================================================================================
#define ERROR(msg) { std::cerr << msg << std::endl; return false; }
bool ParseCommandLine(int argc, const char* argv[], bool* help, vsl::CompileOptions* options)
{
	using namespace vsl;

	*options = {};
	*help = false;

	// Default output file
	const auto inputPath{ fs::absolute(fs::path{ argv[argc - 1] }) };
	const string defaultOutput{ (inputPath.parent_path() / inputPath.stem()).string() + ".vbc" };
	options->outputFile(defaultOutput);

	// Loop over the arguments
	for (uint32 i = 1; i < uint32(argc); ++i) {
		const auto [isFlag, name, param, value] = normalizeArg({ argv[i] });
		if (name.empty()) {
			continue;
		}

		if ((name == "h") || (name == "help")) {
			*help = true;
			return true;
		}
		else if (name == "O") { // Optimization settings
			if (param == "d") {
				options->disableOptimization(true);
			}
			else if (param == "s") {
				options->disableOptimization(false);
			}
			else {
				ERROR(mkstr("Unknown optimization level '%s'", param.c_str()));
			}
		}
		else if (name == "T") { // Binding table sizes
			if (param.empty() || value.empty()) {
				ERROR("Missing name or value for table size argument");
			}
			char* endPtr;
			const auto size = std::strtoul(value.c_str(), &endPtr, 10);
			if (endPtr == value.c_str()) {
				ERROR("Invalid numeric value for table size argument");
			}
			if (size > UINT16_MAX) {
				ERROR("Out-of-range numeric value for table size argument");
			}

			if (param == "samplers") {
				options->tableSizes().samplers = uint16(size);
			}
			else if (param == "images") {
				options->tableSizes().images = uint16(size);
			}
			else if (param == "buffers") {
				options->tableSizes().buffers = uint16(size);
			}
			else if (param == "rotexels") {
				options->tableSizes().roTexels = uint16(size);
			}
			else if (param == "rwtexels") {
				options->tableSizes().rwTexels = uint16(size);
			}
			else {
				ERROR(mkstr("Unknown binding table name '%s'", param.c_str()));
			}
		}
		else if (name == "S") { // Save intermediate artifacts
			if (param == "all") {
				options->saveIntermediate(true);
				options->saveBytecode(true);
			}
			else if (param == "glsl") {
				options->saveIntermediate(true);
			}
			else if (param == "spirv") {
				options->saveBytecode(true);
			}
			else {
				ERROR(mkstr("Intermediate artifact format '%s' not understood", param.c_str()));
			}
		}
		else if (name == "o") {
			if (i >= uint32(argc - 2)) {
				ERROR("No output file specified with -o argument");
			}
			string outFile{ argv[i + 1] };
			options->outputFile(outFile);
			++i;
		}
		else if (name == "no-compile") {
			options->noCompile(true);
		}
		else if (isFlag) {
			std::cout << "Unknown argument '" << name << "' (from " << argv[i] << ")" << std::endl;
		}
	}

	return true;
}

// ====================================================================================================================
void PrintHelp(const char* const arg0)
{
	static const auto TBL_DEF = vsl::CompileOptions::DefaultTableSizes;

	std::cout
		<< "Vega Shader Language Compiler (vslc)\n"
		<< "Usage: " << arg0 << " [options] <file>\n"
		<< "Options:\n"
		<< "    -o <file>         - Set the output file for the compiled shader\n"
		<< "    -Od               - Disable bytecode optimization\n"
		<< "    -Os               - Enable bytecode optimization (default)\n"
		<< "    -T<type>=<value>  - Set the size of the binding table for the given resource type.\n"
		<< "                        Valid types are:\n"
		<< "                            - samplers  (default " << TBL_DEF.samplers << ")\n"
		<< "                            - images    (default " << TBL_DEF.images << ")\n"
		<< "                            - buffers   (default " << TBL_DEF.buffers << ")\n"
		<< "                            - rotexels  (default " << TBL_DEF.roTexels << ")\n"
		<< "                            - rwtexels  (default " << TBL_DEF.rwTexels << ")\n"
		<< "    -S<format>        - Save the intermediate artifact(s) out to files.\n"
		<< "                        Valid formats:\n"
		<< "                            - all    -  Saves all intermediate artifacts.\n"
		<< "                            - glsl   -  Saves the generated GLSL source code.\n"
		<< "                            - spirv  -  Saves the separate SPIR-V modules.\n"
		<< "    --no-compile      - Disable final bytecode compilation and file output.\n"
		<< "                        This will only perform validation on the shader.\n"
		<< std::endl;
}
