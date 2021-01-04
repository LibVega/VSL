/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

/// The main function entry point for the command-line VSL compiler 'vslc'

#include <vsl/Compiler.hpp>

#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;


static bool ParseCommandLine(int argc, char* argv[], bool* help, vsl::CompilerOptions* options);
static void PrintHelp(const char* const arg0);

int main(int argc, char* argv[])
{
	using namespace vsl;

	// No-args check
	if (argc < 2) {
		std::cerr << "Usage: vsl [options] <file>" << std::endl;
		return 1;
	}

	// Try to parse command line
	CompilerOptions options{};
	bool help{ false };
	if (!ParseCommandLine(argc, argv, &help, &options)) {
		return 2;
	}
	if (help) {
		PrintHelp(argv[0]);
		return 0;
	}

	// Run the compilation
	try {
		Compiler c{};
		if (!c.compileFile(argv[argc - 1], options)) {
			const auto err = c.lastError();
			if (err.stage() == CompilerStage::Parse) {
				std::cerr << "Failed to compile (at " << err.line() << ':' << err.character() << ")";
				if (!err.badText().empty()) {
					std::cerr << " ('" << err.badText() << "')";
				}
				std::cerr << " - " << err.message() << std::endl;
			}
			else if (err.stage() == CompilerStage::Generate) {
				std::cerr << "Failed to generate - " << err.message() << std::endl;
			}
			else {
				std::cerr << "Failed to compile - " << err.message() << std::endl;
			}
			return 2;
		}
	}
	catch (const std::exception& ex) {
		std::cerr << "Unhandled exception: " << ex.what() << std::endl;
	}

	return 0;
}


#define ERROR(msg) { std::cerr << msg << std::endl; return false; }

bool ParseCommandLine(int argc, char* argv[], bool* help, vsl::CompilerOptions* options)
{
	using namespace vsl;
	static const auto normalize = [](const string& arg) -> std::tuple<bool, string, string, string> {
		if (arg[0] == '-') {
			const auto isLong = (arg.length() > 1) && (arg[1] == '-');
			const auto valIndex = arg.find('=');
			const auto hasValue = valIndex != string::npos;
			if (arg.length() == (isLong ? 2 : 1)) {
				return std::make_tuple(false, "", "", "");
			}

			if (hasValue) {
				return isLong
					? std::make_tuple(true, arg.substr(2, valIndex - 2), "", arg.substr(valIndex + 1))
					: std::make_tuple(true, string{ arg[1] }, arg.substr(2, valIndex - 2), arg.substr(valIndex + 1));
			}
			else {
				return isLong
					? std::make_tuple(true, arg.substr(2), "", "")
					: std::make_tuple(true, string{ arg[1] }, arg.substr(2), "");
			}
		}
		else {
			return std::make_tuple(false, arg, "", "");
		}
	};

	*options = {};
	*help = false;
	
	// Default output file
	const auto inputPath{ fs::absolute(fs::path{ argv[argc - 1] }) };
	options->outputFile((inputPath.parent_path() / inputPath.stem()).string() + ".vbc");

	// Loop over the arguments
	for (uint32 i = 1; i < uint32(argc); ++i) {
		const auto [isFlag, name, param, value] = normalize({ argv[i] });
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
			options->outputFile(string{ argv[i + 1] });
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

void PrintHelp(const char* const arg0)
{
	std::cout
		<< "Vega Shader Language Compiler (vslc)\n"
		<< "Usage: " << arg0 << " [options] <file>\n"
		<< "Options:\n"
		<< "    -o <file>         - Set the output file for the compiled shader\n"
		<< "    -Od               - Disable bytecode optimization\n"
		<< "    -Os               - Enable bytecode optimization (default)\n"
		<< "    -T<type>=<value>  - Set the size of the binding table for the given resource type.\n"
		<< "                        Valid types are:\n"
		<< "                            - samplers  (default 4096)\n"
		<< "                            - images    (default 512)\n"
		<< "                            - buffers   (default 512)\n"
		<< "                            - rotexels  (default 512)\n"
		<< "                            - rwtexels  (default 512)\n"
		<< "    -S<format>        - Save the intermediate artifact(s) out to files.\n"
		<< "                        Valid formats:\n"
		<< "                            - all    -  Saves all intermediate artifacts.\n"
		<< "                            - glsl   -  Saves the generated GLSL source code.\n"
		<< "                            - spirv  -  Saves the separate SPIR-V modules.\n"
		<< "    --no-compile      - Disable final bytecode compilation and file output.\n"
		<< "                        This will only perform validation on the shader.\n"
		<< std::endl;
}
