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
VISIT_FUNC(File)
{
	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(ShaderTypeStatement)
{
	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(ShaderStructDefinition)
{
	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(ShaderInputOutputStatement)
{
	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(ShaderConstantStatement)
{
	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(ShaderUniformStatement)
{
	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(ShaderBindingStatement)
{
	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(ShaderLocalStatement)
{
	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(ShaderSubpassInputStatement)
{
	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(ShaderStageFunction)
{
	return nullptr;
}

} // namespace vsl
