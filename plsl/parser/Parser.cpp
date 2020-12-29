/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./Parser.hpp"
#include "../../generated/PLSLLexer.h"
#include "../../generated/PLSL.h"

#include <cmath>

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
	, scopes_{ }
	, currentStage_{ }
	, generator_{ &lastError_ }
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

	// Save the generated output
	generator_.saveOutput();
	if (hasError()) {
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
		else if (std::isnan(val) || std::isinf(val)) {
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

// ====================================================================================================================
Variable Parser::parseVariableDeclaration(const grammar::PLSL::VariableDeclarationContext* ctx, bool global)
{
	// Validate name against either the globals, or the current scope tree
	if (ctx->name->getText()[0] == '$') {
		ERROR(ctx->name, "Identifiers starting with '$' are reserved for builtins");
	}
	if ((global && scopes_.hasGlobalName(ctx->name->getText())) || scopes_.hasName(ctx->name->getText())) {
		ERROR(ctx->name, mkstr("Duplicate variable name '%s'", ctx->name->getText().c_str()));
	}
	if (ctx->name->getText()[0] == '_' && *(ctx->name->getText().rbegin()) == '_') {
		ERROR(ctx->name, "Names that start and end with '_' are reserved");
	}

	// Get type
	const auto typeName = ctx->baseType->getText() + (ctx->subType ? '<' + ctx->subType->getText() + '>' : "");
	const auto vType = types_.getOrAddType(typeName);
	if (!vType) {
		ERROR(ctx->baseType, mkstr("Unknown type: %s", types_.lastError().c_str()));
	}
	if (!vType->isComplete()) {
		ERROR(ctx->baseType, mkstr("Incomplete type '%s' (missing subtype specification)", typeName.c_str()));
	}

	// Get array size
	uint32 arrSize = 1;
	if (ctx->arraySize) {
		if (ctx->arraySize->getType() == grammar::PLSL::INTEGER_LITERAL) {
			const auto arrSizeLiteral = ParseLiteral(this, ctx->arraySize);
			if (arrSizeLiteral.isNegative() || arrSizeLiteral.isZero()) {
				ERROR(ctx->arraySize, "Array size cannot be zero or negative");
			}
			if (arrSizeLiteral.u > PLSL_MAX_ARRAY_SIZE) {
				ERROR(ctx->arraySize, "Array size literal is out of range");
			}
			arrSize = uint32(arrSizeLiteral.u);
		}
		else if (ctx->arraySize->getType() == grammar::PLSL::IDENTIFIER) {
			const auto cnst = scopes_.getConstant(ctx->arraySize->getText());
			if (!cnst) {
				ERROR(ctx->arraySize, mkstr("No constant '%s' found for array size", ctx->arraySize->getText().c_str()));
			}
			if (cnst->type == Constant::Float) {
				ERROR(ctx->arraySize, "Cannot use a floating point constant as an array size");
			}
			if (cnst->i <= 0) {
				ERROR(ctx->arraySize, "Array size constant cannot be negative or zero");
			}
			if (cnst->u > PLSL_MAX_ARRAY_SIZE) {
				ERROR(ctx->arraySize, "Array size constant is out of range");
			}
			arrSize = cnst->u;
		}
		else {
			ERROR(ctx->arraySize, "Invalid array literal - compiler bug");
		}
	}

	// Type-specific checks
	if (!vType->isNumeric() && (vType->baseType != ShaderBaseType::Boolean)) { // Handle types
		if (arrSize != 1) {
			ERROR(ctx->arraySize, "Non-numeric types cannot be arrays");
		}
	}

	// Return
	return { VariableType::Unknown, ctx->name->getText(), vType, uint8(arrSize) };
}

// ====================================================================================================================
void Parser::validateSwizzle(uint32 compCount, antlr4::tree::TerminalNode* swizzle) const
{
	if (swizzle->getText().length() > 4) {
		ERROR(swizzle, "Swizzles have a max length of 4");
	}

	for (const auto ch : swizzle->getText()) {
		uint32 idx = UINT32_MAX;
		switch (ch)
		{
		case 'x': case 'r': case 's': idx = 1; break;
		case 'y': case 'g': case 't': idx = 2; break;
		case 'z': case 'b': case 'p': idx = 3; break;
		case 'w': case 'a': case 'q': idx = 4; break;
		}
		if (idx > compCount) {
			ERROR(swizzle, mkstr("Invalid swizzle character '%c' for vector size", ch));
		}
	}
}

} // namespace plsl
