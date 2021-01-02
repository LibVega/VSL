/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#pragma once

#include <vsl/Config.hpp>
#include "../reflection/Types.hpp"
#include "../reflection/ShaderInfo.hpp"

#include <vector>


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


// Represents a specific variable within a program scope
class Variable final
{
public:
	enum Access : uint8 { READONLY, WRITEONLY, READWRITE };

	Variable()
		: type{}, name{ "INVALID" }, dataType{ nullptr }, arraySize{ 0 }, extra{}
	{ }
	Variable(VariableType type, const string& name, const ShaderType* dataType, uint8 arrSize = 1)
		: type{ type }, name{ name }, dataType{ dataType }, arraySize{ arrSize }, extra{}
	{ }

	bool canRead(ShaderStages stage) const;
	bool canWrite(ShaderStages stage) const;

	inline static Variable Builtin(const string& name, const ShaderType* dataType, ShaderStages stage, Access access) {
		Variable var{ VariableType::Builtin, name, dataType, 1 };
		var.extra.builtin.stage = stage;
		var.extra.builtin.access = access;
		return var;
	}

public:
	VariableType type;
	string name;
	const ShaderType* dataType;
	uint8 arraySize;
	union {
		struct {
			ShaderStages pStage;
			bool flat;
		} local;
		struct {
			ShaderStages stage;
			Access access;
		} builtin;
		struct {
			uint8 slot;
		} binding;
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


// Manages a specific variable and name scope within the scope stack
class Scope final
{
public:
	Scope();
	~Scope();

	bool hasName(const string& name) const;

	inline const std::vector<Variable>& variables() const { return variables_; }
	inline std::vector<Variable>& variables() { return variables_; }

private:
	std::vector<Variable> variables_;
}; // class Scope


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

	/* Scopes */
	void pushGlobalScope(ShaderStages stage); // Starts a new scope stack for the given stage
	void pushScope(); // Push a new scope to the stack, must already have an active scope stack
	void popScope();
	bool hasName(const string& name) const; // If the name exists in the current scope stack
	const Variable* getVariable(const string& name) const;
	void addVariable(const Variable& var);

private:
	static void PopulateBuiltins(ShaderStages stage, std::vector<Variable>& vars);

private:
	std::vector<Variable> allGlobals_;
	std::vector<Constant> constants_;
	std::vector<uptr<Scope>> scopes_;

	VSL_NO_COPY(ScopeManager)
	VSL_NO_MOVE(ScopeManager)
}; // class ScopeManager

} // namespace vsl
