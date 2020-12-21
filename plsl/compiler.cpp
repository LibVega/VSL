/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include <plsl/compiler.hpp>

#include <filesystem>
#include <fstream>

#include "./parser/parser.hpp"

#define SET_ERROR(stage,...) lastError_ = CompilerError(CompilerStage::stage, ##__VA_ARGS__);


namespace plsl
{

// ====================================================================================================================
Compiler::Compiler()
	: lastError_{ }
{
	
}

// ====================================================================================================================
Compiler::~Compiler()
{

}

// ====================================================================================================================
bool Compiler::compileFile(const string& path, const CompilerOptions& options) noexcept
{
	// Clear old compile state
	lastError_ = {};

	// Validate the path and file
	std::error_code ioError{};
	const auto inPath = std::filesystem::absolute({ path }, ioError);
	if (ioError) {
		SET_ERROR(FileRead, "Input path is invalid");
		return false;
	}
	if (!std::filesystem::exists(inPath, ioError) || ioError) {
		SET_ERROR(FileRead, "Input file does not exist");
		return false;
	}

	// Try to read the file
	std::ifstream inFile{ inPath.c_str(), std::ifstream::ate | std::istream::in };
	if (!inFile) {
		SET_ERROR(FileRead, "Could not open input file for reading");
		return false;
	}
	const size_t fileLen = inFile.tellg();
	inFile.seekg(0);
	std::stringstream fileContents{};
	fileContents << inFile.rdbuf();

	// Parse the loaded source
	return compileSource(fileContents.str(), options);
}

// ====================================================================================================================
bool Compiler::compileSource(const string& source, const CompilerOptions& options) noexcept
{
	// Perform parsing
	Parser parser{};
	if (!parser.parse(source, options)) {
		lastError_ = parser.lastError();
		return false;
	}

	return true;
}

} // namespace plsl
