/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./Func.hpp"
#include "../reflection/TypeManager.hpp"
#include "../glsl/NameHelper.hpp"

#define ERR_RETURN(msg) { LastError_ = msg; return std::make_tuple(nullptr, ""); }
#define GOOD_RETURN(type,callstr) { LastError_ = ""; return std::make_tuple(type, callstr); }


namespace plsl
{

// ====================================================================================================================
string Functions::LastError_{ };
std::unordered_map<string, std::vector<FunctionEntry>> Functions::Builtins_{ };


// ====================================================================================================================
// ====================================================================================================================
FunctionArg::FunctionArg(const string& typeName, bool gen)
	: type{ TypeManager::GetBuiltinType(typeName) }, genType{ gen && (type->isNumeric() || type->isBoolean()) }
{ }

// ====================================================================================================================
bool FunctionArg::match(const ExprPtr expr) const
{
	const auto etype = expr->type();
	if (genType) {
		const auto casttype = TypeManager::GetNumericType(type->baseType, etype->numeric.dims[0], 1);
		return (etype->isNumeric() || etype->isBoolean()) && !etype->isMatrix() && etype->hasImplicitCast(casttype);
	}
	else {
		return etype->hasImplicitCast(type);
	}
}


// ====================================================================================================================
// ====================================================================================================================
FunctionEntry::FunctionEntry(const string& genName, const string& retTypeName, const std::vector<FunctionArg>& args)
	: genName{ genName }, retType{ TypeManager::GetBuiltinType(retTypeName) }, retIndex{ UINT32_MAX }, args{ args }
{ }

// ====================================================================================================================
const ShaderType* FunctionEntry::match(const std::vector<ExprPtr>& params) const
{
	// Count check
	if (params.size() != args.size()) {
		return nullptr;
	}

	// Per-arg check
	uint32 genSize = 0;
	for (uint32 i = 0; i < args.size(); ++i) {
		const auto& ai = args[i];
		const auto& pi = params[i];
		if (!ai.match(pi)) {
			return nullptr;
		}
		if ((pi->type()->isNumeric() || pi->type()->isBoolean()) && ai.genType) {
			if (genSize == 0) {
				genSize = pi->type()->numeric.dims[0];
			}
			else if (genSize != pi->type()->numeric.dims[0]) {
				return nullptr; // Gen types must match sizes
			}
		}
	}

	return (retIndex == UINT32_MAX)
		? retType
		: TypeManager::GetNumericType(args[retIndex].type->baseType, params[retIndex]->type()->numeric.dims[0], 1);
}


// ====================================================================================================================
// ====================================================================================================================
std::tuple<const ShaderType*, string> Functions::CheckFunction(const string& funcName, 
	const std::vector<ExprPtr>& args)
{
	const auto typeName = TypeManager::GetBuiltinType(funcName);
	if (typeName) {
		return CheckConstructor(funcName, args);
	}
	else {
		if (Builtins_.empty()) {
			Initialize();
		}
		const auto it = Builtins_.find(funcName);
		if (it == Builtins_.end()) {
			ERR_RETURN(mkstr("No function with name '%s' found", funcName.c_str()));
		}
		for (const auto& entry : it->second) {
			const auto match = entry.match(args);
			if (match) {
				GOOD_RETURN(match, entry.genName);
			}
		}
		ERR_RETURN(mkstr("No overload of function '%s' matched the given arguments", funcName.c_str()));
	}
}

// ====================================================================================================================
std::tuple<const ShaderType*, string> Functions::CheckConstructor(const string& typeName, 
	const std::vector<ExprPtr>& args)
{
	// Get the type
	const auto retType = TypeManager::GetBuiltinType(typeName);
	if (!retType) {
		ERR_RETURN(mkstr("No such type '%s' for constructor", typeName.c_str()));
	}
	if (!retType->isNumeric() && !retType->isBoolean()) {
		ERR_RETURN(mkstr("Cannot construct type '%s' - only numeric types have constructors", typeName.c_str()));
	}
	const auto callName = NameHelper::GetGeneralTypeName(retType);

	// Generic argument checks
	if (args.empty()) {
		ERR_RETURN(mkstr("Type '%s' does not have a no-args contructor", typeName.c_str())); // Technically none do
	}
	for (uint32 i = 0; i < args.size(); ++i) {
		if (args[i]->arraySize() != 1) {
			ERR_RETURN(mkstr("Constructor argument %u cannot be an array type", i + 1));
		}
		if (!args[i]->type()->isNumeric() && !args[i]->type()->isBoolean()) {
			ERR_RETURN(mkstr("Constructor argument %u cannot be a non-value type", i + 1));
		}
	}

	// Switch on constructed type
	if (retType->isScalar()) {  // Scalar -> scalar cast
		if (args.size() != 1) {
			ERR_RETURN("Scalar casts cannot have more than one argument");
		}
		if (!args[0]->type()->isScalar()) {
			ERR_RETURN("Scalar casts must take scalar arguments");
		}
		GOOD_RETURN(retType, callName);
	}
	else if (retType->isVector()) {
		const auto ctype = TypeManager::GetNumericType(retType->baseType, 1, 1);
		const auto ccount = uint32(retType->numeric.dims[0]);
		if (args.size() == 1) { 
			const auto a1 = args[0];
			if (a1->type()->isScalar()) {  // Fill vector with scalar
				if (!a1->type()->hasImplicitCast(ctype)) {
					ERR_RETURN(mkstr("Cannot construct type '%s' with scalar type '%s'", 
						retType->getPLSLName().c_str(), a1->type()->getPLSLName().c_str()));
				}
				GOOD_RETURN(retType, callName);
			}
			else if (a1->type()->isVector()) {  // Vector -> vector cast
				if (a1->type()->numeric.dims[0] != ccount) {
					ERR_RETURN("Cannot cast vector types of different component counts");
				}
				GOOD_RETURN(retType, callName);
			}
			else { // Matrix -> vector (invalid)
				ERR_RETURN("Cannot construct vector type from matrix type");
			}
		}
		else { // Standard vector constructor - match component counts
			uint32 found = 0;
			for (uint32 i = 0; i < args.size(); ++i) {
				const auto arg = args[i];
				const auto argCType = TypeManager::GetNumericType(arg->type()->baseType, 1, 1);
				if (arg->type()->isMatrix()) {
					ERR_RETURN(mkstr("Cannot construct vector from matrix argument %u", i + 1));
				}
				if (!argCType->hasImplicitCast(ctype)) {
					ERR_RETURN(mkstr("No implicit cast from argument %u type '%s' to component type '%s'", i + 1,
						argCType->getPLSLName().c_str(), ctype->getPLSLName().c_str()));
				}
				found += arg->type()->numeric.dims[0];
			}
			if (found < ccount) {
				ERR_RETURN(mkstr("Not enough components for %s constructor", retType->getPLSLName().c_str()));
			}
			if (found > ccount) {
				ERR_RETURN(mkstr("Too many components for %s constructor", retType->getPLSLName().c_str()));
			}
			GOOD_RETURN(retType, callName);
		}
	}
	else { // retType->isMatrix()
		const auto ctype = TypeManager::GetNumericType(retType->baseType, 1, 1);
		const auto ccount = uint32(retType->numeric.dims[0]) * retType->numeric.dims[1];
		if (args.size() == 1) {
			const auto a1 = args[1];
			if (a1->type()->isMatrix()) { // Matrix -> matrix (always works for all sizes)
				GOOD_RETURN(retType, callName);
			}
			else if (a1->type()->isVector()) { // Vector -> matrix (never works)
				ERR_RETURN("Cannot construct matrix type from vector type");
			}
			else { // Scalar -> matrix (creates identity matrix with scalar)
				GOOD_RETURN(retType, callName);
			}
		}
		else { // Standard matrix constructor - match component counts
			uint32 found = 0;
			for (uint32 i = 0; i < args.size(); ++i) {
				const auto arg = args[i];
				const auto argCType = TypeManager::GetNumericType(arg->type()->baseType, 1, 1);
				if (!argCType->hasImplicitCast(ctype)) {
					ERR_RETURN(mkstr("No implicit cast from argument %u type '%s' to component type '%s'", i + 1,
						argCType->getPLSLName().c_str(), ctype->getPLSLName().c_str()));
				}
				found += (uint32(arg->type()->numeric.dims[0]) * arg->type()->numeric.dims[1]);
			}
			if (found < ccount) {
				ERR_RETURN(mkstr("Not enough components for %s constructor", retType->getPLSLName().c_str()));
			}
			if (found > ccount) {
				ERR_RETURN(mkstr("Too many components for %s constructor", retType->getPLSLName().c_str()));
			}
			GOOD_RETURN(retType, callName);
		}
	}
}

} // namespace plsl
