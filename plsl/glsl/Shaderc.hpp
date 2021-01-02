/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#pragma once

#include <plsl/Config.hpp>
#include <plsl/Compiler.hpp>
#include "./Generator.hpp"

namespace shaderc { class Compiler; }


namespace plsl
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
	bool compileStage(ShaderStages stage) const;

private:
	std::shared_ptr<shaderc::Compiler> compiler_;
	const CompilerOptions* const options_;
	const Generator* const generator_;
	mutable CompilerError lastError_;

	PLSL_NO_COPY(Shaderc)
	PLSL_NO_MOVE(Shaderc)
}; // class Shaderc

} // namespace plsl
