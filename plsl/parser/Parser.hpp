/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#pragma once

#include <plsl/Config.hpp>
#include <plsl/Compiler.hpp>
#include "./ErrorListener.hpp"
#include "../../generated/PLSLBaseVisitor.h"
#include "../reflection/ShaderInfo.hpp"
#include "../reflection/TypeManager.hpp"
#include "./ScopeManager.hpp"

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


// Result value and code for attempting to parse a literal
struct Literal final
{
public:
	union
	{
		uint64_t u;  // The unsigned parsed literal
		int64_t  i;  // The signed parsed literal
		double   f;  // The floating point parsed literal
	};
	enum : uint8_t
	{
		ENone,        // The literal is valid (no error)
		EOutOfRange,  // The literal is out of range
		EInvalid      // The literal is not parseable
	} parseError;
	enum : uint8_t
	{
		Unsigned,
		Signed,
		Float
	} type;

	Literal()
		: u{ 0 }, parseError{ EInvalid }, type{ Unsigned }
	{ }
	Literal(decltype(parseError) error)
		: u{ 0 }, parseError{ error }, type{ }
	{ }
	Literal(uint64_t val)
		: u{ val }, parseError{ ENone }, type{ Unsigned }
	{ }
	Literal(int64_t val)
		: i{ val }, parseError{ ENone }, type{ Signed }
	{ }
	Literal(double val)
		: f{ val }, parseError{ ENone }, type{ Float }
	{ }

	inline bool isValid() const { return parseError == ENone; }
	inline bool isNegative() const {
		return (type == Float) ? f < 0 : (type == Signed) ? i < 0 : false;
	}
	inline bool isZero() const { return u == 0 || (f == -0); }
}; // struct Literal


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

	/* Utilities */
	static Literal ParseLiteral(const string& txt);
	static Literal ParseLiteral(Parser* parser, const antlr4::Token* token);
	Variable parseVariableDeclaration(const grammar::PLSL::VariableDeclarationContext* ctx, bool global);

	/* File-Level Rules */
	VISIT_DECL(File)
	VISIT_DECL(ShaderTypeStatement)
	VISIT_DECL(ShaderUserTypeDefinition)
	VISIT_DECL(ShaderInputOutputStatement)
	VISIT_DECL(ShaderConstantStatement)
	VISIT_DECL(ShaderBindingStatement)
	VISIT_DECL(ShaderLocalStatement)
	VISIT_DECL(ShaderStageFunction)

private:
	/* Error Functions */
	NORETURN inline void ERROR(const antlr4::Token* tk, const string& msg) {
		throw ParserError(msg, uint32(tk->getLine()), uint32(tk->getCharPositionInLine()));
	}
	NORETURN inline void ERROR(antlr4::RuleContext* ctx, const string& msg) {
		const auto tk = tokens_->get(ctx->getSourceInterval().a);
		throw ParserError(msg, uint32(tk->getLine()), uint32(tk->getCharPositionInLine()));
	}
	NORETURN inline void ERROR(antlr4::tree::TerminalNode* node, const string& msg) {
		const auto tk = tokens_->get(node->getSourceInterval().a);
		throw ParserError(msg, uint32(tk->getLine()), uint32(tk->getCharPositionInLine()));
	}

private:
	ErrorListener errorListener_;
	CompilerError lastError_;
	antlr4::CommonTokenStream* tokens_;
	ShaderInfo shaderInfo_;
	TypeManager types_;
	ScopeManager scopes_;

	PLSL_NO_COPY(Parser)
	PLSL_NO_MOVE(Parser)
}; // class Parser

} // namespace plsl


#undef VISIT_DECL
