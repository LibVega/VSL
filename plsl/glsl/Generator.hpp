/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#pragma once

#include <plsl/Config.hpp>
#include <plsl/Compiler.hpp>
#include "../reflection/ShaderInfo.hpp"

#include <unordered_map>
#include <sstream>


namespace plsl
{

// Produces and manages generated GLSL code 
class Generator final
{
public:
	Generator(CompilerError* error);
	~Generator();

	/* Stages/Functions */
	void setCurrentStage(ShaderStages stage);

	/* Output */
	void saveOutput() const;

private:
	CompilerError* error_;
	std::stringstream globals_; // The global generation (version, extensions, uniforms, push constants)
	std::unordered_map<string, std::stringstream> stageHeaders_;   // The stage-specific headers (in/out)
	std::unordered_map<string, std::stringstream> stageFunctions_; // The function bodies for stage entry points
	std::stringstream* currentFunc_; // The current function being generated
}; // class Generator

} // namespace plsl
