/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./Shader.hpp"
#include "./Parser/Parser.hpp"
#include "./Compiler/Compiler.hpp"

#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;


namespace vsl
{

// ====================================================================================================================
Shader::Shader()
	: options_{ }
	, progress_{ false, false, false }
	, lastError_{ }
	, info_{ }
	, types_{ }
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
		lastError_ = ShaderError("Shader has already parsed VSL source");
		return false;
	}

	// Validate the path and file
	std::error_code ioError{};
	const auto inPath = fs::absolute({ path }, ioError);
	if (ioError) {
		lastError_ = ShaderError("Input path is invalid");
		return false;
	}
	if (!fs::exists(inPath, ioError) || ioError) {
		lastError_ = ShaderError("Input file does not exist");
		return false;
	}

	// Try to read the file
	std::ifstream inFile{ inPath.c_str(), std::istream::in };
	if (!inFile) {
		lastError_ = ShaderError("Could not open input file for reading");
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
		lastError_ = ShaderError("Shader has already parsed VSL source");
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

	// After-parse validation
	if (!bool(info_.stageMask() & ShaderStages::Vertex)) {
		lastError_ = { "Shader is missing required vertex stage", 0, 0 };
		return false;
	}
	if (!bool(info_.stageMask() & ShaderStages::Fragment)) {
		lastError_ = { "Shader is missing required fragment stage", 0, 0 };
		return false;
	}

	progress_.parsed = true;
	return true;
}

// ====================================================================================================================
bool Shader::generate()
{
	// Validate state
	if (!isParsed()) {
		lastError_ = ShaderError("Cannot generate a shader before parsing it");
		return false;
	}
	if (isGenerated()) {
		lastError_ = ShaderError("Shader has already parsed VSL source");
		return false;
	}

	progress_.generated = true;
	return true;
}

// ====================================================================================================================
bool Shader::compile()
{
	// Validate state
	if (!isGenerated()) {
		lastError_ = ShaderError("Cannot compile a shader before generating it");
		return false;
	}
	if (isCompiled()) {
		lastError_ = ShaderError("Shader has already parsed VSL source");
		return false;
	}

	// Create compiler
	Compiler compiler{ this, &options_ };

	// TODO: Compile stages

	// Write final output file
	compiler.writeOutput();

	progress_.compiled = true;
	return true;
}

} // namespace vsl
