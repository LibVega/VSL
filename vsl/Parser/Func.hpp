/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#pragma once

#include "../Config.hpp"
#include "../Types.hpp"

#include <unordered_map>
#include <vector>


namespace vsl
{

class Expr;

// Special type object that can represent a function parameter or return type
// Supports the concept of genType/genIType/genUType/genBType, and out variables
struct FunctionType final
{
public:
	FunctionType() : type{ nullptr }, genType{ false }, refType{ false } { }
	FunctionType(const string& typeName);
	FunctionType(const char* const typeName) : FunctionType(string{ typeName }) { }

	bool match(const SPtr<Expr> expr) const;

public:
	const ShaderType* type; // The argument type (full type or just base type)
	bool genType;
	bool refType; // If the type is an out or inout argument
}; // struct FunctionType


// Describes a specific version (arg list) of a specific function, and how it is emitted to glsl
class FunctionEntry final
{
public:
	FunctionEntry() : genName{ "INVALID" }, retType{}, argTypes{} {}
	FunctionEntry(const string& genName, const string& retTypeName, const std::vector<FunctionType>& args);
	FunctionEntry(const string& genName, const char* const retTypeName, const std::vector<FunctionType>& args)
		: FunctionEntry(genName, string(retTypeName), args)
	{ }

	const ShaderType* match(const std::vector<SPtr<Expr>>& params) const;

public:
	string genName; // Output generated name
	FunctionType retType;
	std::vector<FunctionType> argTypes;
}; // class FunctionEntry


// Contains the registry of built-in and user defined functions, and constructors
class Functions final
{
public:
	/* Function Checks */
	static bool HasFunction(const string& funcName);
	static std::tuple<const ShaderType*, string> CheckFunction(const string& funcName,
		const std::vector<SPtr<Expr>>& args);
	static std::tuple<const ShaderType*, string> CheckConstructor(const string& typeName,
		const std::vector<SPtr<Expr>>& args);

	/* Error */
	inline static bool HasError() { return !LastError_.empty(); }
	inline static const string& LastError() { return LastError_; }

private:
	static void Initialize();

private:
	static string LastError_;
	static std::unordered_map<string, std::vector<FunctionEntry>> Builtins_;
}; // class Functions

} // namespace vsl
