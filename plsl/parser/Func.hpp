/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#pragma once

#include <plsl/Config.hpp>
#include "../reflection/Types.hpp"
#include "../parser/Expr.hpp"

#include <unordered_map>
#include <vector>


namespace plsl
{

// Describes a specific argument for a function entry, with some metadata
struct FunctionArg final
{
public:
	FunctionArg() : type{ nullptr }, genType{ false } { }
	FunctionArg(const ShaderType* type, bool gen = true) 
		: type{ type }, genType{ gen && (type->isNumeric() || type->isBoolean()) }
	{ }
	FunctionArg(const string& typeName, bool gen = true);
	FunctionArg(const char* const typeName, bool gen = true) : FunctionArg(string{ typeName }, gen) { }

	bool match(const ExprPtr expr) const;

public:
	const ShaderType* type;
	bool genType; // If the type is numeric, this matches any scalar/vector of that type
}; // struct FunctionArg


// Describes a specific version (arg list) of a specific function, and how it is emitted to glsl
class FunctionEntry final
{
public:
	FunctionEntry() : genName{ "INVALID" }, retType{}, retIndex{ UINT32_MAX }, args{} {}
	FunctionEntry(const string& genName, const ShaderType* retType, const std::vector<FunctionArg>& args)
		: genName{ genName }, retType{ retType }, retIndex{ UINT32_MAX }, args{ args }
	{ }
	FunctionEntry(const string& genName, const string& retTypeName, const std::vector<FunctionArg>& args);
	FunctionEntry(const string& genName, uint32 retIndex, const std::vector<FunctionArg>& args)
		: genName{ genName }, retType{ nullptr }, retIndex{ retIndex }, args{ args }
	{ }
	FunctionEntry(const char* const genName, uint32 retIndex, const std::vector<FunctionArg>& args)
		: genName{ genName }, retType{ nullptr }, retIndex{ retIndex }, args{ args }
	{ }

	const ShaderType* match(const std::vector<ExprPtr>& params) const;

public:
	string genName; // Output generated name
	const ShaderType* retType; // The return type for the function
	uint32 retIndex; // The genType index to get the return type for
	std::vector<FunctionArg> args;
}; // class FunctionEntry


// Contains the registry of built-in and user defined functions, and constructors
class Functions final
{
public:
	/* Function Checks */
	static std::tuple<const ShaderType*, string> CheckFunction(const string& funcName, 
		const std::vector<ExprPtr>& args);
	static std::tuple<const ShaderType*, string> CheckConstructor(const string& typeName,
		const std::vector<ExprPtr>& args);

	/* Error */
	inline static bool HasError() { return !LastError_.empty(); }
	inline static const string& LastError() { return LastError_; }

private:
	static void Initialize();

private:
	static string LastError_;
	static std::unordered_map<string, std::vector<FunctionEntry>> Builtins_;
}; // class Functions

} // namespace plsl
