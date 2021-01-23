/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#pragma once

#include "../Config.hpp"
#include "../Types.hpp"


namespace vsl
{

// Describes the result of an expression node in a program tree
class Expr final
{
public:
	Expr(const string& refString, const ShaderType* type, uint32 arraySize)
		: refString{ refString }, type{ type }, arraySize{ arraySize }
	{ }

public:
	string refString;
	const ShaderType* type;
	uint32 arraySize;
}; // class Expr

} // namespace vsl
