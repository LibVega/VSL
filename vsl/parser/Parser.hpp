/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#pragma once

#include <vsl/Config.hpp>
#include <vsl/Compiler.hpp>
#include "./ErrorListener.hpp"
#include "../../generated/VSLBaseVisitor.h"
#include "../reflection/ShaderInfo.hpp"
#include "../reflection/TypeManager.hpp"
#include "./ScopeManager.hpp"
#include "../glsl/Generator.hpp"
#include "../glsl/NameHelper.hpp"

#include <antlr4/CommonTokenStream.h>
#include <antlr4/RuleContext.h>
#include <antlr4/Token.h>
#include <antlr4/tree/TerminalNode.h>

#define VISIT_DECL(type) antlrcpp::Any visit##type(grammar::VSL::type##Context* ctx) override;


namespace vsl
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
	public grammar::VSLBaseVisitor
{
	friend class ErrorListener;

public:
	Parser(const CompilerOptions* options);
	virtual ~Parser();

	bool parse(const string& source) noexcept;

	inline const CompilerError& lastError() const { return lastError_; }
	inline bool hasError() const { return !lastError_.message().empty(); }
	inline const ShaderInfo& shaderInfo() const { return shaderInfo_; }
	inline const TypeManager& types() const { return types_; }
	inline const Generator& generator() const { return generator_; }

	/* Utilities */
	static Literal ParseLiteral(const string& txt);
	static Literal ParseLiteral(Parser* parser, const antlr4::Token* token);
	static bool IsValidSwizzle(const string& swizzle);
	Variable parseVariableDeclaration(const grammar::VSL::VariableDeclarationContext* ctx, bool global);
	void validateSwizzle(uint32 compCount, antlr4::tree::TerminalNode* swizzle) const;

	/* File-Level Rules */
	VISIT_DECL(File)
	VISIT_DECL(ShaderTypeStatement)
	VISIT_DECL(ShaderStructDefinition)
	VISIT_DECL(ShaderInputOutputStatement)
	VISIT_DECL(ShaderConstantStatement)
	VISIT_DECL(ShaderUniformStatement)
	VISIT_DECL(ShaderBindingStatement)
	VISIT_DECL(ShaderLocalStatement)
	VISIT_DECL(ShaderStageFunction)

	/* Statements */
	VISIT_DECL(Statement)
	VISIT_DECL(VariableDefinition)
	VISIT_DECL(VariableDeclaration)
	VISIT_DECL(Assignment)
	VISIT_DECL(Lvalue)
	VISIT_DECL(IfStatement)
	VISIT_DECL(ElifStatement)
	VISIT_DECL(ElseStatement)

	/* Expressions */
	VISIT_DECL(FactorExpr)
	VISIT_DECL(NegateExpr)
	antlrcpp::Any visitUnaryOp(const string& optext, grammar::VSL::ExpressionContext* exprCtx);
	VISIT_DECL(MulDivModExpr)
	VISIT_DECL(AddSubExpr)
	VISIT_DECL(ShiftExpr)
	VISIT_DECL(RelationalExpr)
	VISIT_DECL(EqualityExpr)
	VISIT_DECL(BitwiseExpr)
	VISIT_DECL(LogicalExpr)
	antlrcpp::Any visitBinaryOp(const string& optext, grammar::VSL::ExpressionContext* leftCtx, 
		grammar::VSL::ExpressionContext* rightCtx);
	VISIT_DECL(TernaryExpr)
	VISIT_DECL(GroupAtom)
	VISIT_DECL(IndexAtom)
	VISIT_DECL(MemberAtom)
	VISIT_DECL(CallAtom)
	VISIT_DECL(LiteralAtom)
	VISIT_DECL(NameAtom)

private:
	/* Error Functions */
	NORETURN inline void ERROR(const antlr4::Token* tk, const string& msg) const {
		throw ParserError(msg, uint32(tk->getLine()), uint32(tk->getCharPositionInLine()));
	}
	NORETURN inline void ERROR(antlr4::RuleContext* ctx, const string& msg) const {
		const auto tk = tokens_->get(ctx->getSourceInterval().a);
		throw ParserError(msg, uint32(tk->getLine()), uint32(tk->getCharPositionInLine()));
	}
	NORETURN inline void ERROR(antlr4::tree::TerminalNode* node, const string& msg) const {
		const auto tk = tokens_->get(node->getSourceInterval().a);
		throw ParserError(msg, uint32(tk->getLine()), uint32(tk->getCharPositionInLine()));
	}

private:
	const CompilerOptions* options_;
	ErrorListener errorListener_;
	CompilerError lastError_;
	antlr4::CommonTokenStream* tokens_;
	ShaderInfo shaderInfo_;
	TypeManager types_;
	ScopeManager scopes_;
	ShaderStages currentStage_;
	Generator generator_;

	VSL_NO_COPY(Parser)
	VSL_NO_MOVE(Parser)
}; // class Parser

} // namespace vsl


#undef VISIT_DECL
