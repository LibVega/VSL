/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./Func.hpp"
#include "./Parser.hpp"

#define ERR_RETURN(msg) { LastError_ = msg; return std::make_tuple(nullptr, ""); }
#define GOOD_RETURN(type,callstr) { LastError_ = ""; return std::make_tuple(type, callstr); }


namespace vsl
{

// ====================================================================================================================
string Functions::LastError_{ };
std::unordered_map<string, std::vector<FunctionEntry>> Functions::Builtins_{ };


// ====================================================================================================================
// ====================================================================================================================
FunctionType::FunctionType(const string& typeName)
	: type{ nullptr }
	, genType{ false }
	, refType{ false }
{
	// Get out/inout status
	stringview name{};
	if (typeName.find("out ") == 0) {
		name = stringview(typeName).substr(4);
		refType = true;
	}
	else if (typeName.find("inout ") == 0) {
		name = stringview(typeName).substr(6);
		refType = true;
	}
	else {
		name = typeName;
		refType = false;
	}

	// Get the type or gentype
	if (name == "genType") {
		type = TypeList::GetBuiltinType("float");
		genType = true;
	}
	else if (name == "genIType") {
		type = TypeList::GetBuiltinType("int");
		genType = true;
	}
	else if (name == "genUType") {
		type = TypeList::GetBuiltinType("uint");
		genType = true;
	}
	else if (name == "genBType") {
		type = TypeList::GetBuiltinType("bool");
		genType = true;
	}
	else {
		type = TypeList::GetBuiltinType(string(name));
		if (!type) {
			throw std::runtime_error(mkstr("COMPILER BUG - Invalid type name '%s' for function type", name.data()));
		}
		genType = false;
	}
}

// ====================================================================================================================
bool FunctionType::match(const SPtr<Expr> expr) const
{
	const auto etype = expr->type;
	if (expr->arraySize != 1) {
		return false; // No functions take arrays as arguments
	}
	if (genType) {
		const auto casttype = TypeList::GetNumericType(type->baseType, etype->numeric.size, etype->numeric.dims[0], 1);
		return etype->hasImplicitCast(casttype);
	}
	else {
		if (type->isNumericType() || type->isBoolean()) {
			return etype->hasImplicitCast(type);
		}
		else if (type->isSampler() || type->isImage()) {
			return (type->baseType == etype->baseType) && (type->texel.rank == etype->texel.rank);
		}
		else {
			return false;
		}
	}
}


// ====================================================================================================================
// ====================================================================================================================
FunctionEntry::FunctionEntry(const string& genName, const string& retTypeName, const std::vector<FunctionType>& args)
	: genName{ genName }, retType{ retTypeName }, argTypes{ args }
{ }

// ====================================================================================================================
const ShaderType* FunctionEntry::match(const std::vector<SPtr<Expr>>& params) const
{
	// Count check
	if (params.size() != argTypes.size()) {
		return nullptr;
	}

	// Per-arg check
	uint32 genSize = 0;
	uint32 genCount = 0;
	BaseType genType{};
	for (uint32 i = 0; i < argTypes.size(); ++i) {
		const auto& ai = argTypes[i];
		const auto& pi = params[i];
		if (!ai.match(pi)) {
			return nullptr;
		}
		if (ai.genType) {
			if (genSize == 0) {
				genSize = pi->type->numeric.size;
				genCount = pi->type->numeric.dims[0];
				genType = pi->type->baseType;
			}
			else if (genCount != pi->type->numeric.dims[0]) {
				return nullptr; // Gen types must match sizes
			}
		}
	}

	// Return the correct type
	const auto retSize =
		(!retType.genType || (genSize == 0)) ? retType.type->numeric.size :
		(retType.type->baseType != genType) ? retType.type->numeric.size : genSize;
	return retType.genType
		? TypeList::GetNumericType(retType.type->baseType, retSize, genCount, 1)
		: retType.type;
}


// ====================================================================================================================
// ====================================================================================================================
bool Functions::HasFunction(const string& funcName)
{
	if (Builtins_.empty()) {
		Initialize();
	}
	const auto it = Builtins_.find(funcName);
	return (it != Builtins_.end());
}

// ====================================================================================================================
std::tuple<const ShaderType*, string> Functions::CheckFunction(const string& funcName,
	const std::vector<SPtr<Expr>>& args)
{
	const auto typeName = TypeList::GetBuiltinType(funcName);
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
	const std::vector<SPtr<Expr>>& args)
{
	// Get the type
	const auto retType = TypeList::GetBuiltinType(typeName);
	if (!retType) {
		ERR_RETURN(mkstr("No such type '%s' for constructor", typeName.c_str()));
	}
	if (!retType->isNumericType() && !retType->isBoolean()) {
		ERR_RETURN(mkstr("Cannot construct type '%s' - only numeric types have constructors", typeName.c_str()));
	}
	const auto callName = retType->getGLSLName();

	// Generic argument checks
	if (args.empty()) {
		ERR_RETURN(mkstr("Type '%s' does not have a no-args contructor", typeName.c_str())); // Technically none do
	}
	for (uint32 i = 0; i < args.size(); ++i) {
		if (args[i]->arraySize != 1) {
			ERR_RETURN(mkstr("Constructor argument %u cannot be an array type", i + 1));
		}
		if (!args[i]->type->isNumericType() && !args[i]->type->isBoolean()) {
			ERR_RETURN(mkstr("Constructor argument %u cannot be a non-value type", i + 1));
		}
	}

	// Switch on constructed type
	if (retType->isScalar()) {  // Scalar -> scalar cast
		if (args.size() != 1) {
			ERR_RETURN("Scalar casts cannot have more than one argument");
		}
		if (!args[0]->type->isScalar()) {
			ERR_RETURN("Scalar casts must take scalar arguments");
		}
		GOOD_RETURN(retType, callName);
	}
	else if (retType->isVector()) {
		const auto ctype = TypeList::GetNumericType(retType->baseType, retType->numeric.size, 1, 1);
		const auto ccount = uint32(retType->numeric.dims[0]);
		if (args.size() == 1) {
			const auto a1 = args[0];
			if (a1->type->isScalar()) {  // Fill vector with scalar
				if (!a1->type->hasImplicitCast(ctype)) {
					ERR_RETURN(mkstr("Cannot construct type '%s' with scalar type '%s'",
						retType->getVSLName().c_str(), a1->type->getVSLName().c_str()));
				}
				GOOD_RETURN(retType, callName);
			}
			else if (a1->type->isVector()) {  // Vector -> vector cast
				if (a1->type->numeric.dims[0] != ccount) {
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
				const auto argCType = TypeList::GetNumericType(arg->type->baseType, arg->type->numeric.size, 1, 1);
				if (arg->type->isMatrix()) {
					ERR_RETURN(mkstr("Cannot construct vector from matrix argument %u", i + 1));
				}
				if (!argCType->hasImplicitCast(ctype)) {
					ERR_RETURN(mkstr("No implicit cast from argument %u type '%s' to component type '%s'", i + 1,
						argCType->getVSLName().c_str(), ctype->getVSLName().c_str()));
				}
				found += arg->type->numeric.dims[0];
			}
			if (found < ccount) {
				ERR_RETURN(mkstr("Not enough components for %s constructor", retType->getVSLName().c_str()));
			}
			if (found > ccount) {
				ERR_RETURN(mkstr("Too many components for %s constructor", retType->getVSLName().c_str()));
			}
			GOOD_RETURN(retType, callName);
		}
	}
	else { // retType->isMatrix()
		const auto ctype = TypeList::GetNumericType(retType->baseType, retType->numeric.size, 1, 1);
		const auto ccount = uint32(retType->numeric.dims[0]) * retType->numeric.dims[1];
		if (args.size() == 1) {
			const auto a1 = args[1];
			if (a1->type->isMatrix()) { // Matrix -> matrix (always works for all sizes)
				GOOD_RETURN(retType, callName);
			}
			else if (a1->type->isVector()) { // Vector -> matrix (never works)
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
				const auto argCType = TypeList::GetNumericType(arg->type->baseType, arg->type->numeric.size, 1, 1);
				if (!argCType->hasImplicitCast(ctype)) {
					ERR_RETURN(mkstr("No implicit cast from argument %u type '%s' to component type '%s'", i + 1,
						argCType->getVSLName().c_str(), ctype->getVSLName().c_str()));
				}
				found += (uint32(arg->type->numeric.dims[0]) * arg->type->numeric.dims[1]);
			}
			if (found < ccount) {
				ERR_RETURN(mkstr("Not enough components for %s constructor", retType->getVSLName().c_str()));
			}
			if (found > ccount) {
				ERR_RETURN(mkstr("Too many components for %s constructor", retType->getVSLName().c_str()));
			}
			GOOD_RETURN(retType, callName);
		}
	}
}

} // namespace vsl
