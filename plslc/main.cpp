/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

/// The main function entry point for the command-line Polaris compiler 'plslc'

#include <plsl/Compiler.hpp>

#include <iostream>


int main(int argc, char* argv[])
{
	using namespace plsl;

	if (argc < 2) {
		std::cerr << "Usage: plslc <file>" << std::endl;
		return 1;
	}

	Compiler c{};
	if (!c.compileFile(argv[1], {})) {
		const auto err = c.lastError();
		if (err.stage() == CompilerStage::Parse) {
			std::cerr << "Failed to compile (at " << err.line() << ':' << err.character() << ")";
			if (!err.badText().empty()) {
				std::cerr << " ('" << err.badText() << "')";
			}
			std::cerr << " - " << err.message() << std::endl;
		}
		else {
			std::cerr << "Failed to compile - " << err.message() << std::endl;
		}
		return 2;
	}

	return 0;
}
