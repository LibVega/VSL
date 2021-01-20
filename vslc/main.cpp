/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

/// The main function entry point for the command-line VSL compiler 'vslc'

#include "../vsl/Compiler.hpp"

#include <iostream>


bool ParseCommandLine(int argc, const char* argv[], bool* help, vsl::CompilerOptions* options);
void PrintHelp(const char* const arg0);

int main(int argc, char* argv[])
{
	using namespace vsl;

	// No-args check
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " [options] <file>" << std::endl;
		return 1;
	}

	// Try to parse command line
	CompilerOptions options{};
	bool help{ false };
	if (!ParseCommandLine(argc, (const char**)argv, &help, &options)) {
		return 2;
	}
	if (help) {
		PrintHelp(argv[0]);
		return 0;
	}

	// Run the compiler
	try {
		Compiler c{ };
		if (!c.compileFile(argv[argc - 1], options)) {
			const auto& err = c.lastError();
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
			return 3;
		}
	}
	catch (const std::exception& ex) {
		std::cerr << "Unhandled exception: " << ex.what() << std::endl;
		return 4;
	}

	return 0;
}
