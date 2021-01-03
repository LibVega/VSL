/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include <vsl/Compiler.hpp>

#include <filesystem>
#include <fstream>
#include <array>

#include "./parser/Parser.hpp"
#include "./glsl/Shaderc.hpp"

#define SET_ERROR(stage,...) lastError_ = CompilerError(CompilerStage::stage, ##__VA_ARGS__);


namespace vsl
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
	static const std::array<ShaderStages, 5> ALL_STAGES {
		ShaderStages::Vertex, ShaderStages::TessControl, ShaderStages::TessEval, ShaderStages::Geometry,
		ShaderStages::Fragment
	};

	// Perform parsing
	Parser parser{ &options };
	if (!parser.parse(source)) {
		lastError_ = parser.lastError();
		return false;
	}
	const auto& info = parser.shaderInfo();

	// Perform the compilation with shaderc
	Shaderc compiler{ &options, &(parser.generator()) };
	for (const auto stage : ALL_STAGES) {
		if (bool(info.stages() & stage)) {
			if (!compiler.compileStage(stage)) {
				lastError_ = compiler.lastError();
				return false;
			}
		}
	}

	// Write the program
	if (!compiler.writeProgram(info)) {
		lastError_ = compiler.lastError();
		return false;
	}

	return true;
}

} // namespace vsl
