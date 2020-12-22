/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#pragma once

#include <plsl/config.hpp>
#include <plsl/compiler.hpp>
#include "./error_listener.hpp"
#include "../../generated/PLSLBaseVisitor.h"
#include "../reflection/shader_info.hpp"
#include "../reflection/type_manager.hpp"

#include <antlr4/CommonTokenStream.h>
#include <antlr4/RuleContext.h>
#include <antlr4/Token.h>
#include <antlr4/tree/TerminalNode.h>

#define VISIT_DECL(type) antlrcpp::Any visit##type(grammar::PLSL::type##Context* ctx) override;


namespace plsl
{

// The exception type used to stop the tree walk and report an error
class ParserError final
{
public:
	ParserError(const CompilerError& error)
		: error_{ error }
	{ }
	ParserError(const string& msg, uint32 l, uint32 c)
		: error_{ CompilerStage::Parse, msg, l, c }
	{ }

	inline const CompilerError& error() const { return error_; }

private:
	const CompilerError error_;
}; // class ParserError


// The central visitor type that performs the parsing and AST walk
class Parser final :
	public grammar::PLSLBaseVisitor
{
	friend class ErrorListener;

public:
	Parser();
	virtual ~Parser();

	bool parse(const string& source, const CompilerOptions& options) noexcept;

	inline const CompilerError& lastError() const { return lastError_; }
	inline bool hasError() const { return !lastError_.message().empty(); }
	inline const ShaderInfo& shaderInfo() const { return shaderInfo_; }
	inline const TypeManager& types() const { return types_; }

	/* File-Level Rules */
	VISIT_DECL(File)
	VISIT_DECL(ShaderTypeStatement)

private:
	inline void ERROR(antlr4::Token* tk, const string& msg) {
		throw ParserError(msg, uint32(tk->getLine()), uint32(tk->getCharPositionInLine()));
	}
	inline void ERROR(antlr4::RuleContext* ctx, const string& msg) {
		const auto tk = tokens_->get(ctx->getSourceInterval().a);
		throw ParserError(msg, uint32(tk->getLine()), uint32(tk->getCharPositionInLine()));
	}
	inline void ERROR(antlr4::tree::TerminalNode* node, const string& msg) {
		const auto tk = tokens_->get(node->getSourceInterval().a);
		throw ParserError(msg, uint32(tk->getLine()), uint32(tk->getCharPositionInLine()));
	}

private:
	ErrorListener errorListener_;
	CompilerError lastError_;
	antlr4::CommonTokenStream* tokens_;
	ShaderInfo shaderInfo_;
	TypeManager types_;

	PLSL_NO_COPY(Parser)
	PLSL_NO_MOVE(Parser)
}; // class Parser

} // namespace plsl


#undef VISIT_DECL
