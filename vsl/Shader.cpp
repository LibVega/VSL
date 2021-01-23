/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./Shader.hpp"
#include "./Parser/Parser.hpp"

#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

#define SET_ERROR(...) lastError_ = ShaderError(##__VA_ARGS__);


namespace vsl
{

// ====================================================================================================================
Shader::Shader()
	: options_{ }
	, progress_{ false, false, false }
	, lastError_{ }
{

}

// ====================================================================================================================
Shader::~Shader()
{

}

// ====================================================================================================================
bool Shader::parseFile(const string& path, const CompileOptions& options)
{
	// Validate state
	if (isParsed()) {
		SET_ERROR("Shader has already parsed VSL source");
		return false;
	}

	// Validate the path and file
	std::error_code ioError{};
	const auto inPath = fs::absolute({ path }, ioError);
	if (ioError) {
		SET_ERROR("Input path is invalid");
		return false;
	}
	if (!fs::exists(inPath, ioError) || ioError) {
		SET_ERROR("Input file does not exist");
		return false;
	}

	// Try to read the file
	std::ifstream inFile{ inPath.c_str(), std::istream::in };
	if (!inFile) {
		SET_ERROR("Could not open input file for reading");
		return false;
	}
	std::stringstream fileContents{};
	fileContents << inFile.rdbuf();

	// Parse the loaded source
	return parseString(fileContents.str(), options);
}

// ====================================================================================================================
bool Shader::parseString(const string& source, const CompileOptions& options)
{
	// Validate state
	if (isParsed()) {
		SET_ERROR("Shader has already parsed VSL source");
		return false;
	}

	// Save options
	options_ = options;

	// Perform parsing
	Parser parser{ this, &options };
	if (!parser.parse(source)) {
		lastError_ = parser.error();
		return false;
	}

	return true;
}

// ====================================================================================================================
bool Shader::generate()
{
	// Validate state
	if (!isParsed()) {
		SET_ERROR("Cannot generate a shader before parsing it");
		return false;
	}
	if (isGenerated()) {
		SET_ERROR("Shader has already parsed VSL source");
		return false;
	}

	return true;
}

// ====================================================================================================================
bool Shader::compile()
{
	// Validate state
	if (!isGenerated()) {
		SET_ERROR("Cannot compile a shader before generating it");
		return false;
	}
	if (isCompiled()) {
		SET_ERROR("Shader has already parsed VSL source");
		return false;
	}

	return true;
}

} // namespace vsl
