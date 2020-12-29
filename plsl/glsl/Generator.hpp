/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#pragma once

#include <plsl/Config.hpp>
#include <plsl/Compiler.hpp>
#include "../reflection/Types.hpp"
#include "../reflection/ShaderInfo.hpp"

#include <unordered_map>
#include <sstream>


namespace plsl
{

// Reports a fatal error by the generator
class GeneratorError final
{
public:
	GeneratorError(const string& msg)
		: error_{ CompilerStage::Generate, msg, 0, 0 }
	{ }

	inline const CompilerError& error() const { return error_; }

private:
	const CompilerError error_;
}; // class GeneratorError


// Produces and manages generated GLSL code 
class Generator final
{
public:
	Generator();
	~Generator();

	/* Stages/Functions */
	void setCurrentStage(ShaderStages stage);

	/* Output */
	void saveOutput() const;

	/* Global Emit */
	void emitStruct(const string& name, const std::vector<StructMember>& members);
	void emitVertexInput(const InterfaceVariable& var);
	void emitFragmentOutput(const InterfaceVariable& var);
	void emitBinding(const BindingVariable& bind);
	void emitSubpassInput(const SubpassInput& input);

	/* Utilities */
	void getSetAndBinding(const BindingVariable& bind, uint32* set, uint32* binding);

private:
	NORETURN inline void ERROR(const string& msg) const {
		throw GeneratorError(msg);
	}

private:
	std::stringstream globals_; // The global generation (version, extensions, uniforms, push constants)
	std::unordered_map<string, std::stringstream> stageHeaders_;   // The stage-specific headers (in/out)
	std::unordered_map<string, std::stringstream> stageFunctions_; // The function bodies for stage entry points
	std::stringstream* currentFunc_; // The current function being generated
	uint32 uniqueId_; // A unqiue id that can be incremented during generation to give a unique number
	string indentString_; // The current indent level string for function generation

	PLSL_NO_COPY(Generator)
	PLSL_NO_MOVE(Generator)
}; // class Generator

} // namespace plsl
