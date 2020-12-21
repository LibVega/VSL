/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

/// The main function entry point for the command-line Polaris compiler 'plslc'

#include <plsl/compiler.hpp>

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
		std::cerr << "Failed to compile - " << c.lastError().message() << std::endl;
		return 2;
	}

	return 0;
}
