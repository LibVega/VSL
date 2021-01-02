/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#pragma once

#include <plsl/Config.hpp>
#include "../reflection/Types.hpp"


namespace plsl
{

// Contains information about the result of a program expression, such as the type and value text
class Expr final
{
public:
	Expr(const string& ref, const ShaderType* type, uint8 arraySize = 1)
		: refString_{ ref }, type_{ type }, arraySize_{ arraySize }
	{ }

	inline const string& refString() const { return refString_; }
	inline const ShaderType* type() const { return type_; }
	inline uint8 arraySize() const { return arraySize_; }

private:
	string refString_; // The string used to reference the value of the expression (either a literal or variable name)
	const ShaderType* type_;
	uint8 arraySize_;

	PLSL_NO_COPY(Expr)
	PLSL_NO_MOVE(Expr)
}; // class Expr

using ExprPtr = std::shared_ptr<Expr>;

} // namespace plsl
