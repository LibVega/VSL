/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#pragma once

#include <vsl/Config.hpp>
#include "../reflection/Types.hpp"
#include "../parser/Expr.hpp"


namespace vsl
{

// Performs checks and output generation for operation expressions
class Op final
{
public:
	static string ProcessUnaryOp(const string& op, const Expr* e1, const ShaderType** restype);
	static string ProcessBinaryOp(const string& op, const Expr* e1, const Expr* e2, const ShaderType** restype);
	static string ProcessTernaryOp(const Expr* e1, const Expr* e2, const Expr* e3, const ShaderType** restype);

	inline static bool HasError() { return !LastError_.empty(); }
	inline static const string& LastError() { return LastError_; }

private:
	static string LastError_;

	VSL_NO_COPY(Op)
	VSL_NO_MOVE(Op)
}; // class Op

} // namespace vsl
