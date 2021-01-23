/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./Parser.hpp"

#define VISIT_FUNC(type) antlrcpp::Any Parser::visit##type(grammar::VSL::type##Context* ctx)


namespace vsl
{

// ====================================================================================================================
VISIT_FUNC(Statement)
{
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

// ====================================================================================================================
VISIT_FUNC(Lvalue)
{
	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(IfStatement)
{
	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(ElifStatement)
{
	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(ElseStatement)
{
	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(ForLoopStatement)
{
	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(ControlStatement)
{
	return nullptr;
}

} // namespace vsl
