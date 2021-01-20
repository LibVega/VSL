/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

/// The main function entry point for the command-line VSL compiler 'vslc'

#include "../vsl/Shader.hpp"

#include <iostream>


bool ParseCommandLine(int argc, const char* argv[], bool* help, vsl::CompileOptions* options);
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
	CompileOptions options{};
	bool help{ false };
	if (!ParseCommandLine(argc, (const char**)argv, &help, &options)) {
		return 2;
	}
	if (help) {
		PrintHelp(argv[0]);
		return 0;
	}

	// Build the shader
	try {
		Shader shader{};
		if (!shader.parseFile(argv[argc - 1], options)) {
			const auto& err = shader.lastError();
			std::cerr << "Failed to parse [" << err.line() << ':' << err.character() << "]";
			if (!err.badText().empty()) {
				std::cerr << " ('" << err.badText() << "')";
			}
			std::cerr << " - " << err.message() << std::endl;
			return 3;
		}
		if (!shader.generate()) {
			const auto& err = shader.lastError();
			std::cerr << "Failed to generate - " << err.message() << std::endl;
			return 4;
		}
		if (!shader.compile()) {
			const auto& err = shader.lastError();
			std::cerr << "Failed to compile - " << err.message() << std::endl;
			return 5;
		}
	}
	catch (const std::exception& ex) {
		std::cerr << "Unhandled exception: " << ex.what() << std::endl;
		return 6;
	}

	return 0;
}
