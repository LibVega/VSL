/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#pragma once

#include <plsl/config.hpp>

#include <antlr4/BaseErrorListener.h>


namespace plsl
{

// The antlr4 error listener implementation for the PLSL lexer and parser
class ErrorListener final :
	public antlr4::BaseErrorListener
{
public:
	ErrorListener();
	virtual ~ErrorListener();

	void syntaxError(antlr4::Recognizer* recognizer, antlr4::Token* badToken, size_t line, size_t charPosition,
		const std::string& msg, std::exception_ptr e) override;

	PLSL_NO_COPY(ErrorListener)
	PLSL_NO_MOVE(ErrorListener)
}; // class ErrorListener

} // namespace plsl
