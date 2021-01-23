/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#pragma once

#include "../Config.hpp"
#include "../Types.hpp"
#include "../ShaderInfo.hpp"


namespace vsl
{

// The different variable types
enum class VariableType
{
	Unknown,    // Special intermediate type
	Input,      // Vertex input
	Output,     // Fragment output
	Binding,    // Uniform resource binding
	Builtin,    // A stage-specific builtin variable
	Constant,   // Specialization constant
	Local,      // Local value passed between stages
	Parameter,  // Parameter to a function
	Private     // Private within a function
}; // enum class VariableType


// Represents a specific variable within a scope
class Variable final
{
public:
	enum Access : uint8 { READONLY, WRITEONLY, READWRITE };

	Variable()
		: name{ "INVALID" }, varType{ }, dataType{ nullptr }, arraySize{ 0 }, access{ READONLY }, extra{ }
	{ }
	Variable(const string& name, VariableType varType, const ShaderType* dataType, uint32 arraySize, Access access)
		: name{ name }, varType{ varType }, dataType{ dataType }, arraySize{ arraySize }, access{ access }, extra{ }
	{ }

	inline bool canRead() const { return access != WRITEONLY; }
	inline bool canWrite() const { return access != READONLY; }

public:
	string name;
	VariableType varType;
	const ShaderType* dataType;
	uint32 arraySize;
	Access access;
	union {
		struct {
			ShaderStages sourceStage;
			bool flat;
		} local;
		struct {
			uint32 slot;
		} binding;
	} extra;
}; // class Variable

} // namespace vsl
