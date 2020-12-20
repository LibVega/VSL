/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#pragma once

/// The main type for performing the parsing of Polaris shaders

#include <antlr4/CommonTokenStream.h>

#include "../../generated/PLSLBaseVisitor.h"


namespace plsl
{

// The central visitor type that performs the parsing and AST walk to generate shader info
class Visitor final :
	public grammar::PLSLBaseVisitor
{
public:
	Visitor(antlr4::CommonTokenStream* tokens);
	~Visitor();
}; // class Visitor

} // namespace plsl
