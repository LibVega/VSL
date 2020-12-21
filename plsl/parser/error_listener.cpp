/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./error_listener.hpp"


namespace plsl
{

// ====================================================================================================================
ErrorListener::ErrorListener()
{

}

// ====================================================================================================================
ErrorListener::~ErrorListener()
{

}

// ====================================================================================================================
void ErrorListener::syntaxError(antlr4::Recognizer* recognizer, antlr4::Token* badToken, size_t line,
	size_t charPosition, const std::string& msg, std::exception_ptr e)
{

}

} // namespace plsl
