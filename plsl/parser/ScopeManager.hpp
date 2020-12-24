/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#pragma once

#include <plsl/Config.hpp>
#include "../reflection/Types.hpp"
#include "../reflection/ShaderInfo.hpp"

#include <vector>


namespace plsl
{

// The different variable types
enum class VariableType
{
	Unknown,    // Special intermediate type
	Input,      // Vertex input
	Output,     // Fragment output
	Binding,    // Uniform resource binding
	Constant,   // Specialization constant
	Local,      // Local value passed between stages
	Parameter,  // Parameter to a function
	Private     // Private within a function
}; // enum class VariableType


// Represents a specific variable within a program scope
class Variable final
{
public:
	Variable()
		: type{}, name{ "INVALID" }, dataType{ nullptr }, arraySize{ 0 }, extra{}
	{ }
	Variable(VariableType type, const string& name, const ShaderType* dataType, uint8 arrSize = 1)
		: type{ type }, name{ name }, dataType{ dataType }, arraySize{ arrSize }, extra{}
	{ }

public:
	VariableType type;
	string name;
	const ShaderType* dataType;
	uint8 arraySize;
	union {
		struct {
			ShaderStages pStage;
			ShaderStages cStage;
			bool flat;
		} local;
	} extra;
}; // class Variable


// Represents a constant literal
struct Constant final
{
public:
	Constant() : name{ "INVALID" }, u{ 0 }, type{ Unsigned } { }
	Constant(const string& name, uint32 val) : name{ name }, u{ val }, type{ Unsigned } { }
	Constant(const string& name, int32 val) : name{ name }, i{ val }, type{ Signed } { }
	Constant(const string& name, float val) : name{ name }, f{ val }, type{ Float } { }

public:
	string name;
	union
	{
		uint32 u;
		int32 i;
		float f;
	};
	enum
	{
		Unsigned,
		Signed,
		Float
	} type;
}; // struct Constant


// Manages the tree of variable and name scopes that are entered and exited during parsing
class ScopeManager final
{
public:
	ScopeManager();
	~ScopeManager();

	/* Globals */
	bool addGlobal(const Variable& var);
	bool hasGlobal(const string& name) const;
	bool addConstant(const Constant& c);
	bool hasConstant(const string& name) const;
	const Constant* getConstant(const string& name) const;
	bool hasGlobalName(const string& name) const;  // Checks global and constants for used name

private:
	std::vector<Variable> allGlobals_;
	std::vector<Constant> constants_;

	PLSL_NO_COPY(ScopeManager)
	PLSL_NO_MOVE(ScopeManager)
}; // class ScopeManager

} // namespace plsl
