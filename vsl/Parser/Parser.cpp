/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./Parser.hpp"
#include "./ErrorListener.hpp"
#include "../Grammar/VSLLexer.h"


namespace vsl
{

// ====================================================================================================================
Parser::Parser(Shader* shader, const CompileOptions* options)
	: shader_{ shader }
	, options_{ options }
	, error_{ }
	, tokens_{ nullptr }
{

}

// ====================================================================================================================
Parser::~Parser()
{

}

// ====================================================================================================================
bool Parser::parse(const string& source)
{
	// Create the lexer and parser objects
	antlr4::ANTLRInputStream inStream{ source };
	grammar::VSLLexer lexer{ &inStream };
	antlr4::CommonTokenStream tokenStream{ &lexer };
	tokens_ = &tokenStream;
	grammar::VSL parser{ &tokenStream };

	// Install error listener
	ErrorListener listener{ this };
	lexer.removeErrorListeners();
	parser.removeErrorListeners();
	lexer.addErrorListener(&listener);
	parser.addErrorListener(&listener);

	// Perform parsing
	bool result = false;
	const auto fileCtx = parser.file();
	if (hasError()) {
		goto end_parse;
	}

	// Visit the file tree
	try {
		const auto _ = visit(fileCtx);
	}
	catch (const ShaderError& err) {
		error_ = err;
		goto end_parse;
	}
	catch (const std::exception& ex) {
		error_ = { mkstr("Unhandled exception - %s", ex.what()), 0, 0 };
		goto end_parse;
	}

	// Cleanup and return
	result = true;
end_parse:
	tokens_ = nullptr;
	return result;
}

} // namespace vsl
