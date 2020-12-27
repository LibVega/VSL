/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./Parser.hpp"
#include "./Expr.hpp"

#define VISIT_FUNC(type) antlrcpp::Any Parser::visit##type(grammar::PLSL::type##Context* ctx)
#define MAKE_EXPR(name,type,arrSize) (std::make_shared<Expr>(name,type,arrSize))
#define VISIT_EXPR(context) (visit(context).as<std::shared_ptr<Expr>>())


namespace plsl
{

// ====================================================================================================================
VISIT_FUNC(PostfixExpr)
{
	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(PrefixExpr)
{
	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(FactorExpr)
{
	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(NegateExpr)
{
	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(MulDivModExpr)
{
	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(AddSubExpr)
{
	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(ShiftExpr)
{
	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(RelationalExpr)
{
	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(EqualityExpr)
{
	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(BitwiseExpr)
{
	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(LogicalExpr)
{
	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(TernaryExpr)
{
	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(GroupAtom)
{
	return visit(ctx->expression());
}

// ====================================================================================================================
VISIT_FUNC(IndexAtom)
{
	// Visit the left atom and index
	const auto left = VISIT_EXPR(ctx->atom());
	const auto index = VISIT_EXPR(ctx->index);

	// Validate the index
	const auto itype = index->type();
	if ((itype->baseType != ShaderBaseType::UInteger) && (itype->baseType != ShaderBaseType::SInteger)) {
		ERROR(ctx->index, "Array access indices must be integer types");
	}
	if ((index->arraySize() != 1) || (itype->numeric.dims[0] != 1) || (itype->numeric.dims[1] != 1)) {
		ERROR(ctx->index, "Array access indices must be non-array scalar integers");
	}

	// Switch based on left hand type (TODO: Other texture and buffer types)
	if ((left->type()->baseType == ShaderBaseType::ROBuffer) || (left->type()->baseType == ShaderBaseType::RWBuffer)) {
		const auto structType = types_.getType(left->type()->buffer.structName);
		return MAKE_EXPR(mkstr("(%s._data_[%s])", left->refString().c_str(), index->refString().c_str()), 
			structType, 1);
	}
	else if (left->arraySize() != 1) {
		return MAKE_EXPR(mkstr("%s[%s]", left->refString().c_str(), index->refString().c_str()), left->type(), 1);
	}
	else if (left->type()->isNumeric()) {
		if (left->type()->numeric.dims[1] != 1) { // Matrix
			return MAKE_EXPR(mkstr("%s[%s]", left->refString().c_str(), index->refString().c_str()), 
				types_.getNumericType(left->type()->baseType, left->type()->numeric.dims[0], 1), 1);
		}
		else if (left->type()->numeric.dims[0] != 1) { // Vector
			return MAKE_EXPR(mkstr("%s[%s]", left->refString().c_str(), index->refString().c_str()),
				types_.getNumericType(left->type()->baseType, 1, 1), 1);
		}
		else {
			ERROR(ctx->index, "Scalar numeric types cannot have an array indexer applied");
		}
	}
	else if (left->type()->baseType == ShaderBaseType::Boolean) {
		if (left->type()->numeric.dims[0] != 1) {
			return MAKE_EXPR(mkstr("%s[%s]", left->refString().c_str(), index->refString().c_str()),
				types_.getType("bool"), 1);
		}
		else {
			ERROR(ctx->index, "Scalar booleans cannot have an array indexer applied");
		}
	}
	else {
		ERROR(ctx->index, "Type cannot have an array indexer applied");
	}
}

// ====================================================================================================================
VISIT_FUNC(SwizzleAtom)
{
	// Visit left atom
	const auto left = VISIT_EXPR(ctx->atom());

	// Validate the type
	const auto ltype = left->type();
	if (!ltype->isNumeric() || (left->arraySize() != 1)) {
		ERROR(ctx->atom(), "Swizzles can only be applied to non-array numeric types");
	}
	if ((ltype->numeric.dims[0] == 1) || (ltype->numeric.dims[1] != 1)) {
		ERROR(ctx->atom(), "Swizzles can only be applied to vector types");
	}

	// Validate the swizzle
	const auto swtext = ctx->SWIZZLE()->getText();
	validateSwizzle(ltype->numeric.dims[0], ctx->SWIZZLE());

	return MAKE_EXPR(mkstr("%s.%s", left->refString().c_str(), swtext.c_str()), 
		types_.getNumericType(ltype->baseType, uint32(swtext.length()), 1), 1);
}

// ====================================================================================================================
VISIT_FUNC(MemberAtom)
{
	// Visit left atom
	const auto left = VISIT_EXPR(ctx->atom());

	// Validate type (only structs have members)
	const auto ltype = left->type();
	if (!ltype->isStruct()) {
		ERROR(ctx->atom(), "Member access operator '.' can only be applied to struct types");
	}

	// Get the member and type
	const auto memName = ctx->IDENTIFIER()->getText();
	const auto member = ltype->getMember(memName);
	if (!member) {
		ERROR(ctx->IDENTIFIER(), mkstr("Struct type '%s' has no member with name '%s'", 
			ltype->userStruct.structName.c_str(), memName.c_str()));
	}

	return MAKE_EXPR(mkstr("%s.%s", left->refString().c_str(), memName.c_str()), 
		types_.getNumericType(member->baseType, member->dims[0], member->dims[1]), member->arraySize);
}

// ====================================================================================================================
VISIT_FUNC(CallAtom)
{
	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(LiteralAtom)
{
	const auto litptr = ctx->scalarLiteral();

	if (litptr->INTEGER_LITERAL()) {
		const auto text = litptr->INTEGER_LITERAL()->getText();
		const auto literal = ParseLiteral(text);
		if (literal.parseError == Literal::EInvalid) {
			ERROR(litptr->INTEGER_LITERAL(), "Invalid integer literal");
		}
		else if (literal.parseError == Literal::EOutOfRange) {
			ERROR(litptr->INTEGER_LITERAL(), "Integer literal is out of range");
		}
		return MAKE_EXPR((literal.type == Literal::Unsigned) ? mkstr("%llu", literal.u) : mkstr("%lld", literal.i), 
			(literal.type == Literal::Unsigned) ? types_.getType("uint") : types_.getType("int"), 1);
	}
	else if (litptr->FLOAT_LITERAL()) {
		const auto text = litptr->FLOAT_LITERAL()->getText();
		const auto literal = ParseLiteral(text);
		if (literal.parseError == Literal::EInvalid) {
			ERROR(litptr->FLOAT_LITERAL(), "Invalid float literal");
		}
		else if (literal.parseError == Literal::EOutOfRange) {
			ERROR(litptr->FLOAT_LITERAL(), "Float literal is out of range");
		}
		return MAKE_EXPR(mkstr("%f", literal.f), types_.getType("float"), 1);
	}
	else { // litptr->BOOLEAN_LITERAL()
		const auto value = litptr->BOOLEAN_LITERAL()->getText() == "true";
		return MAKE_EXPR(value ? "true" : "false", types_.getType("bool"), 1);
	}
}

// ====================================================================================================================
VISIT_FUNC(NameAtom)
{
	// Find and validate the variable
	const auto varName = ctx->IDENTIFIER()->getText();
	const auto var = scopes_.getVariable(varName);
	if (!var) {
		ERROR(ctx->IDENTIFIER(), mkstr("Could not find variable with name '%s'", varName.c_str()));
	}
	if (!var->canRead(currentStage_)) {
		ERROR(ctx->IDENTIFIER(), mkstr("The variable '%s' is write-only in this context", varName.c_str()));
	}

	// Calculate the correct type
	const ShaderType* type;
	if (var->dataType->baseType == ShaderBaseType::Uniform) {
		type = types_.getType(var->dataType->buffer.structName);
	}
	else {
		type = var->dataType;
	}

	return MAKE_EXPR(var->name, type, var->arraySize);
}

} // namespace plsl
