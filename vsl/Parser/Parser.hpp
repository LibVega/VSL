/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#pragma once

#include "../Shader.hpp"
#include "../Grammar/VSLBaseVisitor.h"

#include <antlr4/CommonTokenStream.h>

#define VISIT_DECL(type) antlrcpp::Any visit##type(grammar::VSL::type##Context* ctx) override;


namespace vsl
{

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

	/* File Level Rules */
	VISIT_DECL(File)
	VISIT_DECL(ShaderTypeStatement)
	VISIT_DECL(ShaderStructDefinition)
	VISIT_DECL(ShaderInputOutputStatement)
	VISIT_DECL(ShaderConstantStatement)
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
	Shader* const shader_;
	const CompileOptions* const options_;
	ShaderError error_;
	antlr4::CommonTokenStream* tokens_;

	VSL_NO_COPY(Parser)
	VSL_NO_MOVE(Parser)
}; // class Parser

} // namespace vsl
