/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./Parser.hpp"

#define VISIT_FUNC(type) antlrcpp::Any Parser::visit##type(grammar::PLSL::type##Context* ctx)


namespace plsl
{

// ====================================================================================================================
VISIT_FUNC(Statement)
{
	// Dispatch
	if (ctx->variableDefinition()) {
		visit(ctx->variableDefinition());
	}
	else if (ctx->variableDeclaration()) {
		visit(ctx->variableDeclaration());
	}
	else if (ctx->assignment()) {
		visit(ctx->assignment());
	}

	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(VariableDefinition)
{
	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(VariableDeclaration)
{
	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(Assignment)
{
	return nullptr;
}

} // namespace plsl
