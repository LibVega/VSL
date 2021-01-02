/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#pragma once

#include <vsl/Config.hpp>
#include <vsl/Compiler.hpp>
#include "../reflection/Types.hpp"
#include "../reflection/ShaderInfo.hpp"
#include "../parser/ScopeManager.hpp"

#include <unordered_map>
#include <sstream>


namespace vsl
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
	Generator(const BindingTableSizes& tableSizes);
	~Generator();

	/* Accessors */
	inline const std::stringstream* globalString() const { return &globals_; }
	bool getStageString(ShaderStages stage, string* outstring) const;

	/* Stages/Functions */
	void setCurrentStage(ShaderStages stage);

	/* Global Emit */
	void emitStruct(const string& name, const std::vector<StructMember>& members);
	void emitVertexInput(const InterfaceVariable& var);
	void emitFragmentOutput(const InterfaceVariable& var);
	void emitBinding(const BindingVariable& bind);
	void emitSubpassInput(const SubpassInput& input);
	void emitLocal(const Variable& var);
	void emitBindingIndices(uint32 maxIndex); // Called on first stage function to finalize the binding indices

	/* Function Emit */
	void emitDeclaration(const Variable& var);
	void emitAssignment(const string& left, const string& op, const string& right);
	void emitImageStore(const string& imStore, const string& value);
	void emitBindingIndex(uint32 index);

	/* Utilities */
	void getSetAndBinding(const BindingVariable& bind, uint32* set, uint32* binding, uint16* tableSize);

private:
	NORETURN inline void ERROR(const string& msg) const {
		throw GeneratorError(msg);
	}

private:
	BindingTableSizes tableSizes_;
	std::stringstream globals_; // The global generation (version, extensions, uniforms, push constants)
	std::unordered_map<string, std::stringstream> stageHeaders_;   // The stage-specific headers (in/out)
	std::unordered_map<string, std::stringstream> stageFunctions_; // The function bodies for stage entry points
	std::stringstream* currentFunc_; // The current function being generated
	uint32 uniqueId_; // A unqiue id that can be incremented during generation to give a unique number
	uint32 localId_; // The monotonically increasing index used to assign local variable locations
	string indentString_; // The current indent level string for function generation
	uint32 bindingEmitMask_; // Mask of binding indices that have been emitted in the current function

	VSL_NO_COPY(Generator)
	VSL_NO_MOVE(Generator)
}; // class Generator

} // namespace vsl
