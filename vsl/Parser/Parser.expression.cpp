/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./Parser.hpp"
#include "../Generator/NameGeneration.hpp"
#include "./Func.hpp"

#define VISIT_FUNC(type) antlrcpp::Any Parser::visit##type(grammar::VSL::type##Context* ctx)
#define MAKE_EXPR(name,type,arrSize) (std::make_shared<Expr>(name,type,arrSize))
#define VISIT_EXPR(context) (visit(context).as<std::shared_ptr<Expr>>())


namespace vsl
{

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
	auto expr = VISIT_EXPR(ctx->expression());
	expr->refString = "(" + expr->refString + ")";
	return expr;
}

// ====================================================================================================================
VISIT_FUNC(IndexAtom)
{
	// Visit components
	const auto left = VISIT_EXPR(ctx->atom());
	const auto& leftStr = left->refString;
	const auto index = VISIT_EXPR(ctx->index);
	const auto& indexStr = index->refString;
	const auto index2 = ctx->index2 ? VISIT_EXPR(ctx->index2) : nullptr;
	const auto& index2Str = index2 ? index2->refString : "";

	// General checks
	if (index2 && !left->type->isMatrix() && !left->type->isSampler()) {
		ERROR(ctx->index2, mkstr("Second indexer is invalid for type '%s'", left->type->getVSLName().c_str()));
	}

	// Switch based on type
	if (left->arraySize != 1) {
		if (index2) {
			ERROR(ctx->index2, "Second indexer not valid for arrays");
		}
		return MAKE_EXPR(mkstr("%s[%s]", leftStr.c_str(), indexStr.c_str()), left->type, 1);
	}
	else if (left->type->isScalar()) {
		ERROR(ctx->atom(), "Indexing is not valid for scalar types");
	}
	else if (left->type->isVector()) {
		if (!index->type->isInteger() || !index->type->isScalar()) {
			ERROR(ctx->index, "Vector indexer must have scalar integer type");
		}
		return MAKE_EXPR(mkstr("%s[%s]", leftStr.c_str(), indexStr.c_str()),
			TypeList::GetNumericType(left->type->baseType, left->type->numeric.size, 1, 1), 1);
	}
	else if (left->type->isMatrix()) {
		if (!index->type->isInteger() || !index->type->isScalar()) {
			ERROR(ctx->index, "Matrix indexer must have scalar integer type");
		}
		if (index2 && (!index2->type->isInteger() || !index2->type->isScalar())) {
			ERROR(ctx->index2, "Second matrix indexer must have scalar integer type");
		}

		if (index2) {
			return MAKE_EXPR(mkstr("%s[%s][%s]", leftStr.c_str(), indexStr.c_str(), index2Str.c_str()),
				TypeList::GetNumericType(left->type->baseType, left->type->numeric.size, 1, 1), 1);
		}
		else {
			return MAKE_EXPR(mkstr("%s[%s]", leftStr.c_str(), indexStr.c_str()),
				TypeList::GetNumericType(left->type->baseType, left->type->numeric.size,
					left->type->numeric.dims[0], 1), 1);
		}
	}
	else if (left->type->isSampler()) {
		const auto compCount = TexelRankGetComponentCount(left->type->texel.rank);
		if (!index->type->isFloat() || (index->type->numeric.dims[0] != compCount)) {
			ERROR(ctx->index, mkstr("Invalid coordinates, %s expects float%u", left->type->getVSLName().c_str(),
				compCount));
		}
		if (index2 && (!index2->type->isInteger() || !index2->type->isScalar())) {
			ERROR(ctx->index2, "Second sampler indexer must have scalar integer type");
		}

		if (index2) {
			return MAKE_EXPR(mkstr("texture(%s, %s, %s)", leftStr.c_str(), indexStr.c_str(), index2Str.c_str()),
				left->type->texel.format->asDataType(), 1);
		}
		else {
			return MAKE_EXPR(mkstr("texture(%s, %s)", leftStr.c_str(), indexStr.c_str()),
				left->type->texel.format->asDataType(), 1);
		}
	}
	else if (left->type->isImage()) {
		const auto compCount = TexelRankGetComponentCount(left->type->texel.rank);
		if (!index->type->isInteger() || (index->type->numeric.dims[0] != compCount)) {
			ERROR(ctx->index, mkstr("Invalid coordinates, %s expects int%u or uint%u",
				left->type->getVSLName().c_str(), compCount, compCount));
		}

		const auto swizzle =
			(left->type->texel.format->count == 1) ? ".x" :
			(left->type->texel.format->count == 2) ? ".xy" : "";
		return MAKE_EXPR(mkstr("(imageLoad(%s, %s)%s)", leftStr.c_str(), indexStr.c_str(), swizzle),
			left->type->texel.format->asDataType(), 1);
	}
	else if (left->type->isROBuffer() || left->type->isRWBuffer()) {
		if (!index->type->isInteger() || !index->type->isScalar()) {
			ERROR(ctx->index, "Buffer indexer must have scalar integer type");
		}

		const auto sType = shader_->types().getType(left->type->buffer.structType->name());
		return MAKE_EXPR(mkstr("%s[%s]", leftStr.c_str(), indexStr.c_str()), sType, 1);
	}
	else if (left->type->isROTexels()) {
		if (!index->type->isInteger() || !index->type->isScalar()) {
			ERROR(ctx->index, "ROTexels indexer must have scalar integer type");
		}

		return MAKE_EXPR(mkstr("texelFetch(%s, %s)", leftStr.c_str(), indexStr.c_str()),
			left->type->texel.format->asDataType(), 1);
	}
	else if (left->type->isRWTexels()) {
		if (!index->type->isInteger() || !index->type->isScalar()) {
			ERROR(ctx->index, "RWTexels indexer must have scalar integer type");
		}

		const auto swizzle =
			(left->type->texel.format->count == 1) ? ".x" :
			(left->type->texel.format->count == 2) ? ".xy" : "";
		return MAKE_EXPR(mkstr("(imageLoad(%s, %s)%s)", leftStr.c_str(), indexStr.c_str(), swizzle),
			left->type->texel.format->asDataType(), 1);
	}
	else {
		ERROR(ctx->atom(), "Invalid type for indexing operations");
	}
}

// ====================================================================================================================
VISIT_FUNC(MemberAtom)
{
	// Visit left atom
	const auto left = VISIT_EXPR(ctx->atom());
	const auto ltype = left->type;
	const auto memberName = ctx->IDENTIFIER()->getText();

	// Switch on type
	if (ltype->isStruct()) {
		// Get the member and type
		const auto member = ltype->userStruct.type->getMember(memberName);
		if (!member) {
			ERROR(ctx->IDENTIFIER(), mkstr("Type '%s' has no member '%s'",
				ltype->userStruct.type->name().c_str(), memberName.c_str()));
		}

		return MAKE_EXPR(mkstr("%s.%s", left->refString.c_str(), memberName.c_str()), member->type, member->arraySize);
	}
	else if (ltype->isVector()) {
		// Validate the swizzle
		validateSwizzle(ltype->numeric.dims[0], ctx->IDENTIFIER());

		return MAKE_EXPR(mkstr("%s.%s", left->refString.c_str(), memberName.c_str()),
			TypeList::GetNumericType(ltype->baseType, ltype->numeric.size, uint32(memberName.length()), 1), 1);
	}
	else {
		ERROR(ctx->atom(), "Operator '.' is only valid for structs (members) or vectors (swizzles)");
	}

	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(CallAtom)
{
	// Visit the argument expressions
	std::vector<SPtr<Expr>> arguments{};
	for (const auto arg : ctx->functionCall()->args) {
		const auto argexpr = VISIT_EXPR(arg);
		arguments.push_back(argexpr);
	}

	// Validate the constructor/function
	const auto fnName = ctx->functionCall()->name->getText();
	const auto [callType, callName] = Functions::CheckFunction(fnName, arguments);
	if (!callType) {
		ERROR(ctx->functionCall()->name, Functions::LastError());
	}

	// Create the call string
	std::stringstream ss{ std::stringstream::out };
	ss << callName << "( ";
	for (const auto& arg : arguments) {
		ss << arg->refString << ", ";
	}
	ss.seekp(-2, std::stringstream::cur);
	ss << " )"; // Overwrite last ", " with function close " )"

	// Emit temp and return
	return MAKE_EXPR(ss.str(), callType, 1);
}

// ====================================================================================================================
VISIT_FUNC(LiteralAtom)
{
	const auto litptr = ctx->scalarLiteral();

	if (litptr->INTEGER_LITERAL()) {
		const auto literal = parseLiteral(litptr->INTEGER_LITERAL()->getSymbol());
		const auto valstr = (literal.type == Literal::Unsigned) ? mkstr("%llu", literal.u) : mkstr("%lld", literal.i);
		const auto type = shader_->types().getType((literal.type == Literal::Unsigned) ? "uint" : "int");

		return MAKE_EXPR(valstr, type, 1);
	}
	else if (litptr->FLOAT_LITERAL()) {
		const auto literal = parseLiteral(litptr->INTEGER_LITERAL()->getSymbol());
		return MAKE_EXPR(mkstr("%f", literal.f), shader_->types().getType("float"), 1);
	}
	else { // litptr->BOOLEAN_LITERAL()
		const auto value = litptr->BOOLEAN_LITERAL()->getText() == "true";
		return MAKE_EXPR(value ? "true" : "false", shader_->types().getType("bool"), 1);
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

	// Calculate the correct expression type and ref string
	const ShaderType* type{};
	string refStr{};
	uint32 arraySize{ 1 };
	if (var->dataType->isNumericType() || var->dataType->isBoolean() || var->dataType->isStruct()) {
		type = var->dataType;
		refStr = var->name;
		arraySize = var->arraySize;
	}
	else if (var->dataType->isSampler() || var->dataType->isImage() || var->dataType->isROTexels() 
			|| var->dataType->isRWTexels()) {
		// Get type/refstr
		type = var->dataType;
		const auto table = NameGeneration::GetBindingTableName(var->dataType);
		refStr = mkstr("(%s[_bidx%u_])", table.c_str(), var->extra.binding.slot);

		// Update binding
		funcGen_->emitBindingIndex(var->extra.binding.slot);
		shader_->info().getBinding(var->extra.binding.slot)->stageMask |= currentStage_;
	}
	else if (var->dataType->isROBuffer() || var->dataType->isRWBuffer()) {
		// Get type/refstr
		type = var->dataType;
		refStr = mkstr("(%s[_bidx%u_]._data_)", var->name.c_str(), var->extra.binding.slot);

		// Update binding
		funcGen_->emitBindingIndex(var->extra.binding.slot);
		shader_->info().getBinding(var->extra.binding.slot)->stageMask |= currentStage_;
	}
	else if (var->dataType->isSPInput()) {
		if (currentStage_ != ShaderStages::Fragment) {
			ERROR(ctx, "Cannot access subpass inputs outside of fragment shader function");
		}
		type = var->dataType->texel.format->asDataType();
		refStr = mkstr("_spi%u_", var->extra.binding.slot);
		
		funcGen_->emitVariableDefinition(type, refStr, mkstr("subpassLoad(%s)", var->name.c_str()));
	}
	else { // Uniform
		type = shader_->types().getType(var->dataType->buffer.structType->name());
		refStr = var->name;
	}

	return MAKE_EXPR(refStr, type, arraySize);
}

} // namespace vsl
