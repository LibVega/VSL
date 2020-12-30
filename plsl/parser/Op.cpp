/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./Op.hpp"

#define ERR_RETURN(msg) { LastError_ = msg; return ""; }
#define CLR_ERROR { LastError_ = ""; }


namespace plsl
{

// ====================================================================================================================
string Op::LastError_{ "" };

// ====================================================================================================================
string Op::ProcessUnaryOp(const string& op, const Expr* e1, const ShaderType** restype)
{
	const auto etype = e1->type();
	const auto& estr = e1->refString();

	// Array check
	if (e1->arraySize() != 1) {
		ERR_RETURN("Cannot apply unary operation to array type");
	}

	// Switch on operator
	if (op == "!") {
		// Type check
		if (etype->baseType != ShaderBaseType::Boolean) {
			ERR_RETURN("Operator '!' requires an operand with a boolean type");
		}
		*restype = e1->type();

		// Return scalar ! or vector not()
		CLR_ERROR;
		return (etype->numeric.dims[0] == 1) ? mkstr("(!%s)", estr.c_str()) : mkstr("(not(%s))", estr.c_str());
	}
	else if (op == "~") {
		// Type check
		if ((etype->baseType != ShaderBaseType::Signed) && (etype->baseType != ShaderBaseType::Unsigned)) {
			ERR_RETURN("Operator '~' requires an operand with an integer type");
		}
		*restype = e1->type();

		// Return '~' (works for all vector sizes)
		CLR_ERROR;
		return mkstr("(~%s)", estr.c_str());
	}
	else { // '+', '-'
		// Type check
		if (!etype->isNumeric()) {
			ERR_RETURN("Operators '+'/'-' require a numeric operand");
		}
		*restype = e1->type();

		// Return '+'/'-' (works for all vector and matrix sizes)
		CLR_ERROR;
		return (op == "-") ? mkstr("(-%s)", estr.c_str()) : estr;
	}
}

// ====================================================================================================================
string Op::ProcessBinaryOp(const string& op, const Expr* e1, const Expr* e2, const ShaderType** restype)
{
	return "";
}

// ====================================================================================================================
string Op::ProcessTernaryOp(const Expr* e1, const Expr* e2, const Expr* e3, const ShaderType** restype)
{
	const auto ctype = e1->type();
	const auto ttype = e2->type();
	const auto ftype = e3->type();
	const auto& cstr = e1->refString();
	const auto& tstr = e2->refString();
	const auto& fstr = e3->refString();

	// Check condition
	if ((ctype->baseType != ShaderBaseType::Boolean) || (ctype->numeric.dims[0] != 1) || (ctype->numeric.dims[1] != 1)) {
		ERR_RETURN("Ternary operator (?:) must have a scalar boolean condition");
	}

	// Check the result types
	if ((e2->arraySize() != 1) || (e3->arraySize() != 1)) {
		ERR_RETURN("Ternary operator cannot take array expressions");
	}
	if (ttype->isNumeric() != ftype->isNumeric()) {
		ERR_RETURN("Incompatible types for ternary expressions");
	}
	if (ttype->baseType == ShaderBaseType::Boolean) {
		if (ftype->baseType != ShaderBaseType::Boolean) {
			ERR_RETURN("False expression for ternary operator is not a boolean type");
		}
		if (ttype->numeric.dims[0] != ftype->numeric.dims[0]) {
			ERR_RETURN("Vector size mismatch for ternary operator expressions");
		}
	}
	else if (!ttype->isNumeric()) {
		if (ttype->baseType != ftype->baseType) {
			ERR_RETURN("Non-numeric type mismatch in ternary operator");
		}
		if (ttype->isStruct() && (ttype->userStruct.structName != ftype->userStruct.structName)) {
			ERR_RETURN("Struct type mismatch in ternary operator");
		}
		if (ttype->baseType != ftype->baseType) {
			ERR_RETURN("Opaque handle type mismatch in ternary operator");
		}
	}
	else { // isNumeric()
		if (!ftype->hasImplicitCast(ttype)) {
			ERR_RETURN("No implicit cast for false expression type to true expression type");
		}
	}

	// Return
	*restype = ttype;
	return mkstr("(%s ? %s : %s)", cstr.c_str(), tstr.c_str(), fstr.c_str());
}

} // namespace plsl
