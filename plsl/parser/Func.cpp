/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./Func.hpp"
#include "../reflection/TypeManager.hpp"

#define ERR_RETURN(msg) { LastError_ = msg; return nullptr; }
#define CLR_ERROR { LastError_ = ""; }


namespace plsl
{

// ====================================================================================================================
string Functions::LastError_{ };

// ====================================================================================================================
const ShaderType* Functions::CheckConstructor(const string& typeName, const std::vector<ExprPtr>& args)
{
	// Get the type
	const auto retType = TypeManager::GetBuiltinType(typeName);
	if (!retType) {
		ERR_RETURN(mkstr("No such type '%s' for constructor", typeName.c_str()));
	}
	if (!retType->isNumeric() && !retType->isBoolean()) {
		ERR_RETURN(mkstr("Cannot construct type '%s' - only numeric types have constructors", typeName.c_str()));
	}

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
		CLR_ERROR;
		return retType;
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
				CLR_ERROR;
				return retType;
			}
			else if (a1->type()->isVector()) {  // Vector -> vector cast
				if (a1->type()->numeric.dims[0] != ccount) {
					ERR_RETURN("Cannot cast vector types of different component counts");
				}
				CLR_ERROR;
				return retType;
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
			CLR_ERROR;
			return retType;
		}
	}
	else { // retType->isMatrix()
		const auto ctype = TypeManager::GetNumericType(retType->baseType, 1, 1);
		const auto ccount = uint32(retType->numeric.dims[0]) * retType->numeric.dims[1];
		if (args.size() == 1) {
			const auto a1 = args[1];
			if (a1->type()->isMatrix()) { // Matrix -> matrix (always works for all sizes)
				CLR_ERROR;
				return retType;
			}
			else if (a1->type()->isVector()) { // Vector -> matrix (never works)
				ERR_RETURN("Cannot construct matrix type from vector type");
			}
			else { // Scalar -> matrix (creates identity matrix with scalar)
				CLR_ERROR;
				return retType;
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
			CLR_ERROR;
			return retType;
		}
	}
}

} // namespace plsl
