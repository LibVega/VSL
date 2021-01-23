/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./Parser.hpp"
#include "./Expr.hpp"
#include "../Generator/NameGeneration.hpp"

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
	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(IndexAtom)
{
	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(MemberAtom)
{
	return nullptr;
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
