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
	// Visit the left atom and indices
	const auto left = VISIT_EXPR(ctx->atom());
	const auto index = VISIT_EXPR(ctx->index);
	const auto index2 = ctx->index2 ? VISIT_EXPR(ctx->index2) : nullptr;

	// Validate the index (not fully, since different types have different index type requirements)
	const auto ltype = left->type();
	const auto itype = index->type();
	const auto i2type = index2 ? index2->type() : nullptr;
	if (!itype->isNumeric()) {
		ERROR(ctx->index, "Index operators must have a numeric type");
	}
	if (i2type && !i2type->isNumeric()) {
		ERROR(ctx->index2, "Index operators must have a numeric type");
	}
	if ((index->arraySize() != 1) || (itype->numeric.dims[1] != 1)) {
		ERROR(ctx->index, "Index operators must be a non-array scalar or vector type");
	}
	if (i2type && (ltype->baseType != ShaderBaseType::Sampler)) {
		ERROR(ctx->index2, "Type does not expect a second indexer");
	}
	if (i2type && ((index2->arraySize() != 1) || (i2type->numeric.dims[1] != 1) || (i2type->numeric.dims[0] != 1))) {
		ERROR(ctx->index2, "Second index operators must be a non-array scalar type");
	}

	// Switch based on left hand type (TODO: Other texture and buffer types)
	if ((ltype->baseType == ShaderBaseType::ROBuffer) || (ltype->baseType == ShaderBaseType::RWBuffer)) {
		const auto structType = types_.getType(ltype->buffer.structName);
		return MAKE_EXPR(mkstr("(%s._data_[%s])", left->refString().c_str(), index->refString().c_str()), 
			structType, 1);
	}
	else if (ltype->baseType == ShaderBaseType::Sampler) { // `sampler*D` -> texture(...)
		const auto dimcount = GetImageDimsComponentCount(ltype->image.dims);
		if (itype->baseType != ShaderBaseType::Float) {
			ERROR(ctx->index, "Sampler type expects floating point indexer");
		}
		if (dimcount != itype->numeric.dims[0]) {
			ERROR(ctx->index, mkstr("Sampler type expects indexer with %u components", dimcount));
		}

		if (ctx->index2) {
			return MAKE_EXPR(
				mkstr("texture(%s, %s, %s)", 
					left->refString().c_str(), index->refString().c_str(), index2->refString().c_str()), 
				types_.getType("float4"), 1);
		}
		else {
			return MAKE_EXPR(mkstr("texture(%s, %s)", left->refString().c_str(), index->refString().c_str()),
				types_.getType("float4"), 1);
		}
	}
	else if (ltype->baseType == ShaderBaseType::Image) { // `image*D` -> imageLoad(...)
		const auto dimcount = GetImageDimsComponentCount(ltype->image.dims);
		if (itype->baseType != ShaderBaseType::Signed && itype->baseType != ShaderBaseType::Unsigned) {
			ERROR(ctx->index, "Image type expects integer indexer");
		}
		if (dimcount != itype->numeric.dims[0]) {
			ERROR(ctx->index, mkstr("Image type expects indexer with %u components", dimcount));
		}

		const auto swizzle =
			(ltype->image.texel.components == 1) ? ".x" :
			(ltype->image.texel.components == 2) ? ".xy" : "";
		return MAKE_EXPR(
			mkstr("(imageLoad(%s, %s)%s)", left->refString().c_str(), index->refString().c_str(), swizzle),
			types_.getNumericType(ltype->image.texel.type, ltype->image.texel.components, 1), 1);
	}
	else if (ltype->baseType == ShaderBaseType::ROTexels) { // `*textureBuffer` -> `texelFetch(...)`
		if (itype->baseType != ShaderBaseType::Signed && itype->baseType != ShaderBaseType::Unsigned) {
			ERROR(ctx->index, "ROTexels type expects integer indexer");
		}
		if (itype->numeric.dims[0] != 1) {
			ERROR(ctx->index, "ROTexels expects scalar indexer");
		}

		return MAKE_EXPR(mkstr("texelFetch(%s, %s)", left->refString().c_str(), index->refString().c_str()),
			types_.getNumericType(ltype->image.texel.type, 4, 1), 1);
	}
	else if (ltype->baseType == ShaderBaseType::RWTexels) { // `imageBuffer` -> imageLoad(...)
		if (itype->baseType != ShaderBaseType::Signed && itype->baseType != ShaderBaseType::Unsigned) {
			ERROR(ctx->index, "RWTexels type expects integer indexer");
		}
		if (itype->numeric.dims[0] != 1) {
			ERROR(ctx->index, "RWTexels expects scalar indexer");
		}

		const auto swizzle =
			(ltype->image.texel.components == 1) ? ".x" :
			(ltype->image.texel.components == 2) ? ".xy" : "";
		return MAKE_EXPR(
			mkstr("(imageLoad(%s, %s)%s)", left->refString().c_str(), index->refString().c_str(), swizzle),
			types_.getNumericType(ltype->image.texel.type, ltype->image.texel.components, 1), 1);
	}
	else if (left->arraySize() != 1) {
		return MAKE_EXPR(mkstr("%s[%s]", left->refString().c_str(), index->refString().c_str()), ltype, 1);
	}
	else if (ltype->isNumeric()) {
		if (ltype->numeric.dims[1] != 1) { // Matrix
			return MAKE_EXPR(mkstr("%s[%s]", left->refString().c_str(), index->refString().c_str()), 
				types_.getNumericType(ltype->baseType, ltype->numeric.dims[0], 1), 1);
		}
		else if (ltype->numeric.dims[0] != 1) { // Vector
			return MAKE_EXPR(mkstr("%s[%s]", left->refString().c_str(), index->refString().c_str()),
				types_.getNumericType(ltype->baseType, 1, 1), 1);
		}
		else {
			ERROR(ctx->index, "Scalar numeric types cannot have an array indexer applied");
		}
	}
	else if (ltype->baseType == ShaderBaseType::Boolean) {
		if (ltype->numeric.dims[0] != 1) {
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
	string refStr{};
	if (var->dataType->baseType == ShaderBaseType::Uniform) {
		type = types_.getType(var->dataType->buffer.structName);
		refStr = var->name;
	}
	else if (var->dataType->baseType == ShaderBaseType::Input) {
		type = types_.getNumericType(var->dataType->image.texel.type, 4, 1);
		refStr = mkstr("subpassLoad(%s)", var->name.c_str());
	}
	else {
		type = var->dataType;
		refStr = var->name;
	}

	return MAKE_EXPR(refStr, type, var->arraySize);
}

} // namespace plsl
