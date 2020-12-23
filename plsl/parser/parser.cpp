/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./parser.hpp"
#include "../../generated/PLSLLexer.h"
#include "../../generated/PLSL.h"

#define SET_ERROR(stage,...) lastError_ = CompilerError(CompilerStage::stage, ##__VA_ARGS__);


namespace plsl
{

// ====================================================================================================================
Parser::Parser()
	: errorListener_{ this }
	, lastError_{ }
	, tokens_{ nullptr }
	, shaderInfo_{ }
	, types_{ }
{
	
}

// ====================================================================================================================
Parser::~Parser()
{

}

// ====================================================================================================================
bool Parser::parse(const string& source, const CompilerOptions& options) noexcept
{
	// Create the lexer object
	antlr4::ANTLRInputStream inStream{ source };
	grammar::PLSLLexer lexer{ &inStream };
	lexer.removeErrorListeners();
	lexer.addErrorListener(&errorListener_);

	// Create the parser object
	antlr4::CommonTokenStream tokenStream{ &lexer };
	tokens_ = &tokenStream;
	grammar::PLSL parser{ &tokenStream };
	parser.removeErrorListeners();
	parser.addErrorListener(&errorListener_);

	// Perform the lexing and parsing
	const auto fileCtx = parser.file();
	bool result = false;
	if (hasError()) {
		goto end_parse; // The error listener in the lexer/parser picked up an error
	}

	// Visit the parsed file context
	try {
		const auto any = this->visit(fileCtx);
	}
	catch (const ParserError& pe) {
		lastError_ = pe.error();
		goto end_parse;
	}
	catch (const std::exception& ex) {
		SET_ERROR(Parse, mkstr("Unhanded error - %s", ex.what()));
		goto end_parse;
	}
	result = true;

end_parse:
	// Cleanup and return
	tokens_ = nullptr;
	return result;
}

// ====================================================================================================================
Literal Parser::ParseLiteral(const string& txt)
{
	if (txt.empty()) {
		return { Literal::EInvalid };
	}

	const char* beg = txt.data();
	char* end;

	// Parse floats separately
	const bool isFlt =
		(txt.find('.') != string::npos) ||
		(txt.find('e') != string::npos) ||
		(txt.find('E') != string::npos);
	if (isFlt) {
		const auto val = std::strtod(beg, &end);
		if (errno == ERANGE) {
			return { Literal::EOutOfRange };
		}
		else if (isnan(val) || isinf(val)) {
			return { Literal::EOutOfRange };
		}
		else if (end == beg) {
			return { Literal::EInvalid };
		}
		else {
			return { val };
		}
	}

	// Check neg state
	const bool isNeg = txt[0] == '-';
	const bool isHex = txt.find("0x") == 0;

	// Parse the value as 64-bit integers
	if (isHex || !isNeg) {
		const auto val = std::strtoull(beg, &end, isHex ? 16 : 10);
		if (errno == ERANGE) {
			return { Literal::EOutOfRange };
		}
		else if (end == beg) {
			return { Literal::EInvalid };
		}
		else {
			return { uint64_t(val) };
		}
	}
	else {
		const auto val = std::strtoll(beg, &end, 10);
		if (errno == ERANGE) {
			return { Literal::EOutOfRange };
		}
		else if (end == beg) {
			return { Literal::EInvalid };
		}
		else {
			return { int64_t(val) };
		}
	}
}

// ====================================================================================================================
Literal Parser::ParseLiteral(Parser* parser, const antlr4::Token* token)
{
	const auto lit = ParseLiteral(token->getText());
	if (lit.parseError == Literal::EOutOfRange) {
		parser->ERROR(token, mkstr("Numeric literal '%s' is out of range", token->getText().c_str()));
	}
	else if (lit.parseError == Literal::EInvalid) {
		parser->ERROR(token, mkstr("Numeric literal '%s' is invalid", token->getText().c_str()));
	}
	else {
		return lit;
	}
}

} // namespace plsl
