/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./error_listener.hpp"
#include "./parser.hpp"

#define STRMATCH(mstr) (msg.find(mstr)!=string::npos) 


namespace plsl
{

// ====================================================================================================================
ErrorListener::ErrorListener(Parser* parser)
	: parser_{ parser }
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
	using namespace antlr4;

	// Extract extra error information from the exception
	const RuleContext* ctx{ nullptr };
	string badText = badToken ? badToken->getText() : "";
	if (e) {
		try { std::rethrow_exception(e); }
		catch (const RecognitionException& ex) {
			ctx = ex.getCtx();
			if (badText.empty() && ex.getOffendingToken()) {
				badText = ex.getOffendingToken()->getText();
			}
		}
	}
	const size_t ruleIdx = ctx ? ctx->getRuleIndex() : SIZE_MAX;

	// The customized error text (TODO: Expand this by exhaustively testing the different errors)
	string errorMsg{};

	// Check all of the known error combinations
	if (STRMATCH("expecting ';'")) {
		errorMsg = "expected end of statement ';'";
	}
	else if (STRMATCH("missing INTEGER_LITERAL at ']'")) {
		errorMsg = "missing array size specifier";
	}
	else {
		// Fallback for errors we have not customized a response to
		errorMsg = mkstr("(Rule '%s') (Bad Text: '%s') - %s",
			(ruleIdx == SIZE_MAX) ? "none" : recognizer->getRuleNames()[ruleIdx].c_str(),
			badText.c_str(), msg.c_str());
	}

	// Set the parser error
	parser_->lastError_ = CompilerError(CompilerStage::Parse, errorMsg, uint32(line), uint32(charPosition));
	if (!badText.empty()) {
		parser_->lastError_.badText(badText);
	}
}

} // namespace plsl
