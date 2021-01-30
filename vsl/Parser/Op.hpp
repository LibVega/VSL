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

// Special type object that can represent an operation parameter or return type
// Supports the concept of genType/genIType/genUType/genBType
struct OpType final
{
public:
	OpType() : type{ nullptr }, genType{ false } { }
	OpType(const string& typeName);
	OpType(const char* const typeName) : OpType(string{ typeName }) { }

	bool match(const SPtr<Expr> expr) const;

public:
	const ShaderType* type; // The argument type (full type or just base type)
	bool genType;
}; // struct OpType


// Describes a specific version (operand list) of a specific operation, and how it is emitted to glsl
class OpEntry final
{
public:
	OpEntry() : genStr{ "INVALID" }, retType{}, argTypes{} {}
	OpEntry(const string& genStr, const string& retTypeName, const std::vector<OpType>& args);
	OpEntry(const string& genStr, const char* const retTypeName, const std::vector<OpType>& args)
		: OpEntry(genStr, string(retTypeName), args)
	{ }

	const ShaderType* match(const std::vector<SPtr<Expr>>& params) const;
	string generateString(const string& op, const std::vector<SPtr<Expr>>& params) const;

public:
	string genStr; // Output generated string (with $1, $2, $3 for operands and $op for operator)
	OpType retType;
	std::vector<OpType> argTypes;
}; // class OpEntry


// Contains the registry of operators
class Ops final
{
public:
	/* Operator Checks */
	static std::tuple<const ShaderType*, string> CheckOp(const string& op,
		const std::vector<SPtr<Expr>>& args);

	/* Error */
	inline static bool HasError() { return !LastError_.empty(); }
	inline static const string& LastError() { return LastError_; }

private:
	static void Initialize();

private:
	static string LastError_;
	static std::unordered_map<string, std::vector<OpEntry>> Ops_;
}; // class Ops

} // namespace vsl
