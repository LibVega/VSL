/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#pragma once

#include <vsl/Config.hpp>
#include <vsl/Compiler.hpp>
#include "./Generator.hpp"

#include <unordered_map>

namespace shaderc { class Compiler; }


namespace vsl
{

// Wraps an interface to a shaderc compilation task
class Shaderc final
{
public:
	Shaderc(const CompilerOptions* options, const Generator* generator);
	~Shaderc();

	/* Error */
	inline bool hasError() const { return lastError_.stage() == CompilerStage::Compile; }
	inline const CompilerError& lastError() const { return lastError_; }

	/* Compilation */
	bool compileStage(ShaderStages stage);
	bool writeProgram(const ShaderInfo& info);

private:
	/* File Output */
	bool writeIntermediate(ShaderStages stage, const string& source);
	bool writeBytecode(ShaderStages stage);

private:
	std::shared_ptr<shaderc::Compiler> compiler_;
	const CompilerOptions* const options_;
	const Generator* const generator_;
	mutable CompilerError lastError_;
	ShaderStages stages_;
	std::unordered_map<ShaderStages, std::vector<uint32>> bytecodes_;

	VSL_NO_COPY(Shaderc)
	VSL_NO_MOVE(Shaderc)
}; // class Shaderc

} // namespace vsl
