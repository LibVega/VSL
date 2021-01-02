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


static bool ParseCommandLine(int argc, char* argv[], vsl::CompilerOptions* options);

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
	if (!ParseCommandLine(argc, argv, &options)) {
		return 2;
	}

	// Run the compilation
	try {
		Compiler c{};
		if (!c.compileFile(argv[1], options)) {
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

bool ParseCommandLine(int argc, char* argv[], vsl::CompilerOptions* options)
{
	*options = {};
	
	// Default output file
	const auto inputPath{ fs::absolute(fs::path{ argv[argc - 1] }) };
	options->outputFile((inputPath.parent_path() / inputPath.stem()).string() + ".vsp");

	return true;
}
