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

	inline bool canRead(ShaderStages stage) const { 
		return (varType == VariableType::Local) ? (stage == ShaderStages::Fragment) : (access != WRITEONLY);
	}
	inline bool canWrite(ShaderStages stage) const { 
		return (varType == VariableType::Local) ? (stage == ShaderStages::Vertex) : (access != READONLY);
	}

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


// Manages a specific variable and name scope within the scope stack
class Scope final
{
public:
	enum ScopeType {
		Function,     // Function scope
		Conditional,  // If/elif/else scope
		Loop          // Looping scope
	};

	Scope(ScopeType type = Function);
	~Scope();

	bool hasName(const string& name) const;

	inline ScopeType type() const { return type_; }
	inline const std::vector<Variable>& variables() const { return variables_; }
	inline std::vector<Variable>& variables() { return variables_; }

private:
	ScopeType type_;
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
	bool hasGlobalName(const string& name) const;  // Checks global and constants for used name

	/* Scopes */
	void pushGlobalScope(ShaderStages stage); // Starts a new scope stack for the given stage
	void pushScope(Scope::ScopeType type); // Push a new scope to the stack, must already have an active scope stack
	void popScope();
	bool hasName(const string& name) const; // If the name exists in the current scope stack
	const Variable* getVariable(const string& name) const;
	void addVariable(const Variable& var);
	bool inLoop() const; // If the scope stack contains a loop scope at any depth

private:
	static void PopulateBuiltins(ShaderStages stage, std::vector<Variable>& vars);

private:
	std::vector<Variable> allGlobals_;
	std::vector<UPtr<Scope>> scopes_;

	VSL_NO_COPY(ScopeManager)
	VSL_NO_MOVE(ScopeManager)
}; // class ScopeManager

} // namespace vsl
