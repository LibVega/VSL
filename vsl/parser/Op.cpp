/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./Op.hpp"
#include "../reflection/TypeManager.hpp"

#define ERR_RETURN(msg) { LastError_ = msg; return ""; }
#define CLR_ERROR { LastError_ = ""; }


namespace vsl
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
		if (etype->isBoolean()) {
			ERR_RETURN("Operator '!' requires an operand with a boolean type");
		}
		*restype = e1->type();

		// Return scalar ! or vector not()
		CLR_ERROR;
		return (etype->numeric.dims[0] == 1) ? mkstr("(!%s)", estr.c_str()) : mkstr("(not(%s))", estr.c_str());
	}
	else if (op == "~") {
		// Type check
		if (!etype->isInteger()) {
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
	const auto ltype = e1->type();
	const auto rtype = e2->type();
	const auto& lstr = e1->refString();
	const auto& rstr = e2->refString();

	// Basic checks
	if ((e1->arraySize() != 1) || (e2->arraySize() != 1)) {
		ERR_RETURN("Binary operators cannot take arrays as operands");
	}
	if (!ltype->isNumeric() && !ltype->isBoolean()) {
		ERR_RETURN("Binary operators cannot take non-numeric types");
	}
	if (!rtype->isNumeric() && !rtype->isBoolean()) {
		ERR_RETURN("Binary operators cannot take non-numeric types");
	}
	if (ltype->isNumeric() != rtype->isNumeric()) {
		ERR_RETURN("Cannot mix numeric and boolean operands for binary operators");
	}

	// Special rules for multiplication
	if (op == "*") {
		if (!ltype->isNumeric()) {
			ERR_RETURN("Cannot apply mathematic operators to boolean types");
		}

		if (ltype->isMatrix()) { // Left hand matrix
			if (rtype->isMatrix()) { // Matrix * Matrix
				if (ltype->numeric.dims[1] != rtype->numeric.dims[0]) {
					ERR_RETURN("Incompatible sizes for matrix multiplication");
				}
				*restype = TypeManager::GetNumericType(ltype->baseType, ltype->numeric.dims[0], rtype->numeric.dims[1]);
			}
			else if (rtype->isVector()) { // Matrix * Vector
				if (ltype->numeric.dims[1] != rtype->numeric.dims[0]) {
					ERR_RETURN("Incompatible sizes for matrix/vector multiplication");
				}
				*restype = rtype;
			}
			else { // Matrix * Scalar
				*restype = ltype;
			}
		}
		else if (ltype->isVector()) { // Left hand vector
			if (rtype->isMatrix()) { // Vector * Matrix
				ERR_RETURN("Cannot multiple vector by matrix (must be matrix * vector)");
			}
			else if (rtype->isVector()) { // Vector * Vector
				const auto isUnsigned =
					(ltype->baseType == ShaderBaseType::Unsigned) || (rtype->baseType == ShaderBaseType::Unsigned);
				const auto isFloat = ltype->isFloat() || rtype->isFloat();
				*restype = TypeManager::GetNumericType(
					isFloat ? ShaderBaseType::Float : isUnsigned ? ShaderBaseType::Unsigned : ShaderBaseType::Signed,
					ltype->numeric.dims[0], 1);
			}
			else { // Vector * Scalar
				*restype = ltype;
			}
		}
		else { // Left hand scalar
			if (rtype->isMatrix()) { // Scalar * matrix
				*restype = rtype;
			}
			else if (rtype->isVector()) { // Scalar * Vector
				const auto isUnsigned =
					(ltype->baseType == ShaderBaseType::Unsigned) || (rtype->baseType == ShaderBaseType::Unsigned);
				const auto isFloat = ltype->isFloat() || rtype->isFloat();
				*restype = TypeManager::GetNumericType(
					isFloat ? ShaderBaseType::Float : isUnsigned ? ShaderBaseType::Unsigned : ShaderBaseType::Signed,
					ltype->numeric.dims[0], 1);
			}
			else { // Scalar * Scalar
				const auto isUnsigned =
					(ltype->baseType == ShaderBaseType::Unsigned) || (rtype->baseType == ShaderBaseType::Unsigned);
				const auto isFloat = ltype->isFloat() || rtype->isFloat();
				*restype = TypeManager::GetNumericType(
					isFloat ? ShaderBaseType::Float : isUnsigned ? ShaderBaseType::Unsigned : ShaderBaseType::Signed,
					1, 1);
			}
		}

		CLR_ERROR;
		return mkstr("(%s * %s)", lstr.c_str(), rstr.c_str());
	}

	// Check casting, then switch on operator groups
	if (ltype->isNumeric() && !rtype->hasImplicitCast(ltype)) {
		ERR_RETURN("No implicit cast from right operand type to left operand type");
	}
	if ((op == "/") || (op == "+") || (op == "-")) {
		if (ltype->isBoolean()) {
			ERR_RETURN("Cannot apply mathematic operators to boolean types");
		}
		*restype = ltype;
		CLR_ERROR;
		return mkstr("(%s %s %s)", lstr.c_str(), op.c_str(), rstr.c_str());
	}
	else if (op == "%") {
		if (ltype->isBoolean()) {
			ERR_RETURN("Cannot apply mod operator to boolean types");
		}
		*restype = ltype;
		CLR_ERROR;
		return (ltype->numeric.dims[0] != 1)
			? mkstr("mod(%s, %s)", lstr.c_str(), rstr.c_str())
			: mkstr("(%s %% %s)", lstr.c_str(), rstr.c_str());
	}
	else if ((op == "<<") || (op == ">>")) {
		if (!ltype->isInteger()) {
			ERR_RETURN("Bitshift operator can only accept integer types");
		}
		if (ltype->baseType != rtype->baseType) {
			ERR_RETURN("Bitshift operands must match signed-ness");
		}
		if (ltype->isMatrix()) {
			ERR_RETURN("Bitshift operator cannot accept matrix types");
		}
		*restype = ltype;
		CLR_ERROR;
		return mkstr("(%s %s %s)", lstr.c_str(), op.c_str(), rstr.c_str());
	}
	else if ((op == "<") || (op == ">") || (op == "<=") || (op == ">=")) {
		if (!ltype->isNumeric()) {
			ERR_RETURN("Relational operators can only accept numeric types");
		}
		if (ltype->isMatrix()) {
			ERR_RETURN("Relational operators cannot accept matrix operands");
		}
		*restype = ltype->isVector()
			? TypeManager::GetBuiltinType(mkstr("bool%u", uint32(ltype->numeric.dims[0])))
			: TypeManager::GetBuiltinType("bool");
		const string funcName =
			(op == "<")  ? "lessThan" :
			(op == ">")  ? "greaterThan" :
			(op == "<=") ? "lessThanEqual" : "greaterThanEqual";
		CLR_ERROR;
		return ltype->isVector()
			? mkstr("%s(%s, %s)", funcName.c_str(), lstr.c_str(), rstr.c_str())
			: mkstr("(%s %s %s)", lstr.c_str(), op.c_str(), rstr.c_str());
	}
	else if ((op == "==") || (op == "!=")) {
		if (ltype->isMatrix()) {
			ERR_RETURN("Equality operators cannot accept matrix operands");
		}
		*restype = ltype->isVector()
			? TypeManager::GetBuiltinType(mkstr("bool%u", uint32(ltype->numeric.dims[0])))
			: TypeManager::GetBuiltinType("bool");
		const string funcName = (op == "==") ? "equal" : "notEqual";
		CLR_ERROR;
		return ltype->isVector()
			? mkstr("%s(%s, %s)", funcName.c_str(), lstr.c_str(), rstr.c_str())
			: mkstr("(%s %s %s)", lstr.c_str(), op.c_str(), rstr.c_str());
	}
	else if ((op == "&") || (op == "|") || (op == "^")) {
		if (!ltype->isInteger()) {
			ERR_RETURN("Bitwise operators require integer type operands");
		}
		const auto isUnsigned =
			(ltype->baseType == ShaderBaseType::Unsigned) || (rtype->baseType == ShaderBaseType::Unsigned);
		*restype = TypeManager::GetNumericType(isUnsigned ? ShaderBaseType::Unsigned : ShaderBaseType::Signed,
			ltype->numeric.dims[0], 1);
		CLR_ERROR;
		return mkstr("(%s %s %s)", lstr.c_str(), op.c_str(), rstr.c_str());
	}
	else if ((op == "&&") || (op == "||")) {
		if (!ltype->isBoolean()) {
			ERR_RETURN("Logical operators require boolean operands");
		}
		if (!ltype->isScalar()) {
			ERR_RETURN("Logical operators require scalar boolean operands");
		}
		*restype = TypeManager::GetBuiltinType("bool");
		CLR_ERROR;
		return mkstr("(%s %s %s)", lstr.c_str(), op.c_str(), rstr.c_str());
	}
	else {
		ERR_RETURN(mkstr("Invalid binary operator for types '%s' and '%s'", ltype->getVSLName().c_str(),
			rtype->getVSLName().c_str()));
	}
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
	if (!ctype->isBoolean() || !ctype->isScalar() || (e1->arraySize() != 1)) {
		ERR_RETURN("Ternary operator must have a scalar boolean condition");
	}

	// Check the result types
	if ((e2->arraySize() != 1) || (e3->arraySize() != 1)) {
		ERR_RETURN("Ternary operator cannot take array expressions");
	}
	if (ttype->isNumeric() != ftype->isNumeric()) {
		ERR_RETURN("Incompatible types for ternary expressions");
	}
	if (ttype->isBoolean()) {
		if (ftype->isBoolean()) {
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
			ERR_RETURN(mkstr("No implicit cast for false expression '%s' to true expression '%s'",
				ftype->getVSLName().c_str(), ttype->getVSLName().c_str()));
		}
	}

	// Return
	*restype = ttype;
	return mkstr("(%s ? %s : %s)", cstr.c_str(), tstr.c_str(), fstr.c_str());
}

} // namespace vsl
