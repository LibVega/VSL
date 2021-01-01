/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#pragma once

#include <plsl/Config.hpp>
#include "../reflection/Types.hpp"
#include "../parser/Expr.hpp"


namespace plsl
{

// Contains the registry of built-in and user defined functions
class Functions final
{
public:
	/* Function Checks */
	static const ShaderType* CheckConstructor(const string& typeName, const std::vector<ExprPtr>& args);

	/* Error */
	inline static bool HasError() { return !LastError_.empty(); }
	inline static const string& LastError() { return LastError_; }

private:
	static string LastError_;
}; // class Functions

} // namespace plsl
