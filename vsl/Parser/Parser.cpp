/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./Parser.hpp"
#include "./ErrorListener.hpp"
#include "../Grammar/VSLLexer.h"
#include "./Func.hpp"

#include <cmath>


namespace vsl
{

// ====================================================================================================================
Parser::Parser(Shader* shader, const CompileOptions* options)
	: shader_{ shader }
	, options_{ options }
	, error_{ }
	, tokens_{ nullptr }
	, scopes_{ }
	, currentStage_{ ShaderStages::None }
	, funcGen_{ nullptr }
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

// ====================================================================================================================
void Parser::validateName(const antlr4::Token* name)
{
	const auto varName = name->getText();
	if (varName[0] == '$') {
		ERROR(name, "Identifiers starting with '$' are reserved for builtin variables");
	}
	if (varName.length() > Shader::MAX_NAME_LENGTH) {
		ERROR(name, mkstr("Variable names cannot be longer than %u bytes", Shader::MAX_NAME_LENGTH));
	}
	if ((varName[0] == '_') && (*varName.rbegin() == '_')) {
		ERROR(name, "Names that start and end with '_' are reserved");
	}
	if (shader_->types().getType(varName)) {
		ERROR(name, mkstr("Variable name '%s' overlaps with type name", varName.c_str()));
	}
	if (scopes_.hasGlobalName(varName) || scopes_.hasName(varName)) {
		ERROR(name, mkstr("Duplicate variable name '%s'", varName.c_str()));
	}
	if (Functions::HasFunction(varName)) {
		ERROR(name, mkstr("Variable name '%s' overlaps with function name", varName.c_str()));
	}
	if (shader_->types().getType(varName)) {
		ERROR(name, mkstr("Variable name '%s' overlaps with type name", varName.c_str()));
	}
}

// ====================================================================================================================
Variable Parser::parseVariableDeclaration(const grammar::VSL::VariableDeclarationContext* ctx, bool global)
{
	// Perform name validation
	validateName(ctx->name);

	// Get type
	const auto typeName = ctx->baseType->getText() + (ctx->subType ? '<' + ctx->subType->getText() + '>' : "");
	const auto vType = shader_->types().parseOrGetType(typeName);
	if (!vType) {
		ERROR(ctx->baseType, mkstr("Unknown type: %s", shader_->types().lastError().c_str()));
	}

	// Get array size
	uint32 arrSize = 1;
	if (ctx->arraySize) {
		const auto arrSizeLiteral = parseLiteral(ctx->arraySize);
		if (arrSizeLiteral.isNegative() || arrSizeLiteral.isZero()) {
			ERROR(ctx->arraySize, "Array size cannot be zero or negative");
		}
		if (arrSizeLiteral.u > Shader::MAX_ARRAY_SIZE) {
			ERROR(ctx->arraySize, mkstr("Array is larger than max allowed size %u", Shader::MAX_ARRAY_SIZE));
		}
		arrSize = uint32(arrSizeLiteral.u);
	}

	// Type-specific checks
	if (!vType->isNumericType() && !vType->isBoolean()) { // Handle types
		if (arrSize != 1) {
			ERROR(ctx->arraySize, "Non-numeric types cannot be arrays");
		}
	}

	// Return
	return { ctx->name->getText(), VariableType::Unknown, vType, arrSize, Variable::READWRITE };
}

// ====================================================================================================================
Literal Parser::parseLiteral(const antlr4::Token* token)
{
	const auto txt = token->getText();
	const char* beg = txt.data();
	char* end;

	// Basic checks
	if (txt.empty()) {
		ERROR(token, "Cannot parse empty literal");
	}

	// Try parse float first
	const bool isFlt = (txt.find_first_of(".eE", 0) != string::npos);
	if (isFlt) {
		const auto val = std::strtod(beg, &end);
		if (errno == ERANGE) {
			ERROR(token, "Floating point literal is outside representable range");
		}
		else if (std::isnan(val) || std::isinf(val)) {
			ERROR(token, "Floating point literal cannot be NaN or inf");
		}
		else if (end == beg) {
			ERROR(token, "Invalid floating point literal");
		}
		else {
			return { val };
		}
	}

	// Check integer components
	const bool isNeg = (txt[0] == '-');
	const bool isHex = (txt[0] == '0') && (txt.length() > 1) && (std::tolower(txt[1]) == 'x');
	const bool isU = std::tolower(*txt.rbegin()) == 'u';

	// Parse integers
	if (isHex || isU) {
		if (isNeg) {
			ERROR(token, "Cannot negate hex or unsigned integer literal");
		}
		const auto val = std::strtoull(beg, &end, isHex ? 16 : 10);
		if (errno == ERANGE) {
			ERROR(token, "Unsigned integer literal is outside representable range");
		}
		else if (end == beg) {
			ERROR(token, "Invalid unsigned integer literal");
		}
		else {
			return { uint64(val) };
		}
	}
	else {
		const auto val = std::strtoll(beg, &end, 10);
		if (errno == ERANGE) {
			ERROR(token, "Signed integer literal is outside representable range");
		}
		else if (end == beg) {
			ERROR(token, "Invalid signed integer literal");
		}
		else {
			return { int64(val) };
		}
	}
}

// ====================================================================================================================
void Parser::validateSwizzle(uint32 compCount, antlr4::tree::TerminalNode* swizzle)
{
	// Check length
	const auto swtxt = swizzle->getText();
	if (swtxt.length() > 4) {
		ERROR(swizzle, "Swizzles have a max length of 4");
	}

	// Check each character
	uint32 cclass = UINT32_MAX;
	for (const auto ch : swizzle->getText()) {
		uint32 idx = UINT32_MAX;
		uint32 cc = UINT32_MAX;

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

		switch (ch)
		{
		case 'x': case 'y': case 'z': case 'w': cc = 1; break;
		case 'r': case 'g': case 'b': case 'a': cc = 2; break;
		case 's': case 't': case 'p': case 'q': cc = 3; break;
		}
		if (cclass != UINT32_MAX) {
			if (cc != cclass) {
				const auto expect = (cclass == 1) ? "xyzw" : (cclass == 2) ? "rgba" : "stpq";
				ERROR(swizzle, mkstr("Swizzle class mismatch for character '%c', expected one of '%s'", ch, expect));
			}
		}
		else {
			cclass = cc;
		}
	}
}

} // namespace vsl
