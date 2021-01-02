/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include <vsl/Compiler.hpp>

#include <filesystem>
#include <fstream>

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
	// Perform parsing
	Parser parser{ &options };
	if (!parser.parse(source)) {
		lastError_ = parser.lastError();
		return false;
	}
	const auto& info = parser.shaderInfo();

	// Perform the compilation with shaderc
	Shaderc compiler{ &options, &(parser.generator()) };
	if (bool(info.stages() & ShaderStages::Vertex)) {
		if (!compiler.compileStage(ShaderStages::Vertex)) {
			lastError_ = compiler.lastError();
			return false;
		}
	}
	if (bool(info.stages() & ShaderStages::TessControl)) {
		if (!compiler.compileStage(ShaderStages::TessControl)) {
			lastError_ = compiler.lastError();
			return false;
		}
	}
	if (bool(info.stages() & ShaderStages::TessEval)) {
		if (!compiler.compileStage(ShaderStages::TessEval)) {
			lastError_ = compiler.lastError();
			return false;
		}
	}
	if (bool(info.stages() & ShaderStages::Geometry)) {
		if (!compiler.compileStage(ShaderStages::Geometry)) {
			lastError_ = compiler.lastError();
			return false;
		}
	}
	if (bool(info.stages() & ShaderStages::Fragment)) {
		if (!compiler.compileStage(ShaderStages::Fragment)) {
			lastError_ = compiler.lastError();
			return false;
		}
	}

	return true;
}

} // namespace vsl
