/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#pragma once

#include <plsl/Config.hpp>


namespace plsl
{

// Contains information about the result of a program expression, such as the type and value text
class Expr final
{
	PLSL_NO_COPY(Expr)
	PLSL_NO_MOVE(Expr)
}; // class Expr

using ExprPtr = std::shared_ptr<Expr>;

} // namespace plsl
