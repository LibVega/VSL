/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#pragma once

#include "../Shader.hpp"
#include "../Grammar/VSLBaseVisitor.h"
#include "./ScopeManager.hpp"

#include <antlr4/CommonTokenStream.h>
#include <antlr4/RuleContext.h>
#include <antlr4/Token.h>
#include <antlr4/tree/TerminalNode.h>

#define VISIT_DECL(type) antlrcpp::Any visit##type(grammar::VSL::type##Context* ctx) override;


namespace vsl
{

// Result value and code for attempting to parse a literal
struct Literal final
{
public:
	Literal() : u{ 0 }, type{ Unsigned } { }
	Literal(uint64_t val) : u{ val }, type{ Unsigned } { }
	Literal(int64_t val) : i{ val }, type{ Signed } { }
	Literal(double val) : f{ val }, type{ Float } { }

	inline bool isNegative() const {
		return (type == Float) ? f < 0 : (type == Signed) ? i < 0 : false;
	}
	inline bool isZero() const { return u == 0 || (f == -0); }

public:
	union {
		uint64 u;  // The unsigned parsed literal
		int64  i;  // The signed parsed literal
		double f;  // The floating point parsed literal
	};
	enum : uint32 {
		Unsigned,
		Signed,
		Float
	} type;
}; // struct Literal


// Central ANTLR parser type for VSL programs
class Parser final
	: public grammar::VSLBaseVisitor
{
	friend class ErrorListener;

public:
	Parser(Shader* shader, const CompileOptions* options);
	~Parser();

	bool parse(const string& source);

	/* Error */
	inline const ShaderError& error() const { return error_; }
	inline bool hasError() const { return !error_.message().empty(); }

	/* Utilities */
	void validateName(const antlr4::Token* name);
	Variable parseVariableDeclaration(const grammar::VSL::VariableDeclarationContext* ctx, bool global);
	Literal parseLiteral(const antlr4::Token* token);

	/* File Level Rules */
	VISIT_DECL(File)
	VISIT_DECL(ShaderTypeStatement)
	VISIT_DECL(ShaderStructDefinition)
	VISIT_DECL(ShaderInputOutputStatement)
	VISIT_DECL(ShaderUniformStatement)
	VISIT_DECL(ShaderBindingStatement)
	VISIT_DECL(ShaderLocalStatement)
	VISIT_DECL(ShaderSubpassInputStatement)
	VISIT_DECL(ShaderStageFunction)

	/* Statement Rules */
	VISIT_DECL(Statement)
	VISIT_DECL(VariableDefinition)
	VISIT_DECL(VariableDeclaration)
	VISIT_DECL(Assignment)
	VISIT_DECL(Lvalue)
	VISIT_DECL(IfStatement)
	VISIT_DECL(ElifStatement)
	VISIT_DECL(ElseStatement)
	VISIT_DECL(ForLoopStatement)
	VISIT_DECL(ControlStatement)

	/* Expression Rules */
	VISIT_DECL(FactorExpr)
	VISIT_DECL(NegateExpr)
	VISIT_DECL(MulDivModExpr)
	VISIT_DECL(AddSubExpr)
	VISIT_DECL(ShiftExpr)
	VISIT_DECL(RelationalExpr)
	VISIT_DECL(EqualityExpr)
	VISIT_DECL(BitwiseExpr)
	VISIT_DECL(LogicalExpr)
	VISIT_DECL(TernaryExpr)
	VISIT_DECL(GroupAtom)
	VISIT_DECL(IndexAtom)
	VISIT_DECL(MemberAtom)
	VISIT_DECL(CallAtom)
	VISIT_DECL(LiteralAtom)
	VISIT_DECL(NameAtom)

private:
	/* Error */
	NORETURN inline void ERROR(const antlr4::Token* tk, const string& msg) const {
		ShaderError err{ msg, uint32(tk->getLine()), uint32(tk->getCharPositionInLine()) };
		err.badText(tk->getText());
		throw err;
	}
	NORETURN inline void ERROR(antlr4::RuleContext* ctx, const string& msg) const {
		const auto tk = tokens_->get(ctx->getSourceInterval().a);
		ShaderError err{ msg, uint32(tk->getLine()), uint32(tk->getCharPositionInLine()) };
		err.badText(tk->getText());
		throw err;
	}
	NORETURN inline void ERROR(antlr4::tree::TerminalNode* node, const string& msg) const {
		const auto tk = tokens_->get(node->getSourceInterval().a);
		ShaderError err{ msg, uint32(tk->getLine()), uint32(tk->getCharPositionInLine()) };
		err.badText(tk->getText());
		throw err;
	}

private:
	Shader* const shader_;
	const CompileOptions* const options_;
	ShaderError error_;
	antlr4::CommonTokenStream* tokens_;
	ScopeManager scopes_;
	ShaderStages currentStage_;

	VSL_NO_COPY(Parser)
	VSL_NO_MOVE(Parser)
}; // class Parser

} // namespace vsl
