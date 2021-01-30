/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./Parser.hpp"
#include "../Generator/NameGeneration.hpp"
#include "./Op.hpp"

#define VISIT_FUNC(type) antlrcpp::Any Parser::visit##type(grammar::VSL::type##Context* ctx)
#define VISIT_EXPR(context) (visit(context).as<std::shared_ptr<Expr>>())
#define MAKE_EXPR(name,type,arrSize) (std::make_shared<Expr>(name,type,arrSize))


namespace vsl
{

// ====================================================================================================================
template <typename T> 
inline constexpr int signum(T val) {
	return (T(0) < val) - (val < T(0));
}


// ====================================================================================================================
VISIT_FUNC(Statement)
{
	// Dispatch
	if (ctx->variableDefinition()) {
		visit(ctx->variableDefinition());
	}
	else if (ctx->variableDeclaration()) {
		visit(ctx->variableDeclaration());
	}
	else if (ctx->assignment()) {
		visit(ctx->assignment());
	}
	else if (ctx->ifStatement()) {
		visit(ctx->ifStatement());
	}
	else if (ctx->forLoopStatement()) {
		visit(ctx->forLoopStatement());
	}
	else if (ctx->controlStatement()) {
		visit(ctx->controlStatement());
	}

	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(VariableDefinition)
{
	// Create the variable definition
	const auto varDecl = ctx->variableDeclaration();
	auto var = parseVariableDeclaration(varDecl, false);
	if (var.arraySize != 1) {
		ERROR(varDecl->arraySize, "Function-local variables cannot be arrays");
	}
	if (!var.dataType->isNumericType() && !var.dataType->isBoolean()) {
		ERROR(varDecl->baseType, "Function-local variable must be numeric or boolean type");
	}

	// Visit and check expression
	const auto expr = VISIT_EXPR(ctx->value);
	const auto etype = expr->type;
	if (!etype->hasImplicitCast(var.dataType)) {
		ERROR(ctx->value, mkstr("No implicit cast from '%s' to '%s'", etype->getVSLName().c_str(),
			var.dataType->getVSLName().c_str()));
	}

	// Add the variable
	var.varType = VariableType::Private;
	scopes_.addVariable(var);

	// Emit declaration and assignment
	const auto typeStr = var.dataType->getGLSLName();
	funcGen_->emitAssignment(typeStr + " " + var.name, "=", expr->refString);

	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(VariableDeclaration)
{
	// Create the variable definition
	auto var = parseVariableDeclaration(ctx, false);
	if (var.arraySize != 1) {
		ERROR(ctx->arraySize, "Function-local variables cannot be arrays");
	}
	if (!var.dataType->isNumericType() && !var.dataType->isBoolean()) {
		ERROR(ctx->baseType, "Function-local variable must be numeric or boolean type");
	}

	// Add the variable
	var.varType = VariableType::Private;
	scopes_.addVariable(var);

	// Emit declaration and assignment
	funcGen_->emitDeclaration(var.dataType, var.name);

	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(Assignment)
{
	// Visit the left-hand side
	const auto left = VISIT_EXPR(ctx->lval);
	const auto ltype = left->type;
	const auto isImageStore = (left->refString.find("imageStore") == 0);

	// Visit expression
	const auto expr = VISIT_EXPR(ctx->value);
	const auto etype = expr->type;

	// Validate types
	const auto optxt = ctx->op->getText();
	const auto isCompound = (optxt != "=");
	if (isImageStore) {
		if (isCompound) {
			ERROR(ctx->op, "Compound assignment not allowed on Image or RWTexel types");
		}
		if ((etype->baseType != ltype->baseType) || (etype->numeric.dims[0] != ltype->numeric.dims[0]) ||
			etype->isMatrix()) {
			ERROR(ctx->op, mkstr("Cannot store type '%s' in object with texel type '%s'", etype->getVSLName().c_str(),
				ltype->getVSLName().c_str()));
		}

		// Get the value (need to promote the stored type to *gvec4* for imageStore(...))
		const auto dims = etype->numeric.dims[0];
		const string prefix = etype->isSigned() ? "i" : etype->isUnsigned() ? "u" : "";
		const auto valstr =
			(dims == 1) ? mkstr("%svec4(%s, 0, 0, 0)", prefix.c_str(), expr->refString.c_str()) :
			(dims == 2) ? mkstr("%svec4(%s, 0, 0)", prefix.c_str(), expr->refString.c_str()) : expr->refString;

		// Emit image store
		funcGen_->emitImageStore(left->refString, valstr);
	}
	else if (!isCompound) {
		if (!etype->hasImplicitCast(left->type)) {
			ERROR(ctx->value, mkstr("No implicit cast from '%s' to '%s'", etype->getVSLName().c_str(),
				left->type->getVSLName().c_str()));
		}
		funcGen_->emitAssignment(left->refString, optxt, expr->refString);
	}
	else {
		const auto subop = optxt.substr(0, optxt.length() - 1);
		const auto [resType, refStr] = Ops::CheckOp(subop, { left, expr });
		if (!resType) {
			ERROR(ctx->value, mkstr("Compound assignment '%s' not possible with types '%s' and '%s'",
				optxt.c_str(), ltype->getVSLName().c_str(), etype->getVSLName().c_str()));
		}
		funcGen_->emitAssignment(left->refString, optxt, expr->refString);
	}

	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(Lvalue)
{
	// Switch on lvalue type
	if (ctx->name) {
		// Find the variable
		const auto varName = ctx->name->getText();
		const auto var = scopes_.getVariable(varName);
		if (!var) {
			ERROR(ctx->name, mkstr("No variable with name '%s' found", varName.c_str()));
		}
		if (!var->canWrite(currentStage_)) {
			ERROR(ctx->name, mkstr("The variable '%s' is read-only in this context", varName.c_str()));
		}

		// Get the name of the variable based on the variable type
		string outname{};
		switch (var->varType)
		{
		case VariableType::Binding: {
			if (var->dataType->baseType == BaseType::Uniform) {
				outname = var->name;
				if (shader_->info().hasUniform()) {
					shader_->info().stageMask(shader_->info().stageMask());
				}
			}
			else if (var->dataType->isBufferType()) {
				funcGen_->emitBindingIndex(var->extra.binding.slot);
				outname = mkstr("%s[_bidx%u_]", var->name.c_str(), uint32(var->extra.binding.slot));
				shader_->info().getBinding(var->extra.binding.slot)->stageMask |= currentStage_;
			}
			else {
				const auto table = NameGeneration::GetBindingTableName(var->dataType);
				funcGen_->emitBindingIndex(var->extra.binding.slot);
				outname = mkstr("(%s[_bidx%u_])", table.c_str(), uint32(var->extra.binding.slot));
				shader_->info().getBinding(var->extra.binding.slot)->stageMask |= currentStage_;
			}
		} break;
		case VariableType::Builtin: {
			outname = NameGeneration::GetGLSLBuiltinName(var->name);
		} break;
		case VariableType::Local: {
			outname = mkstr("_%s_%s", ShaderStageToStr(currentStage_).c_str(), varName.c_str());
		} break;
		default: outname = varName;
		}

		return MAKE_EXPR(outname, var->dataType, var->arraySize);
	}
	else if (ctx->index) {
		// Get the lvalue
		const auto left = VISIT_EXPR(ctx->val);
		const auto ltype = left->type;
		if (left->refString.find("imageStore") == 0) {
			ERROR(ctx->val, "Image or RWTexel stores must be top-level lvalue");
		}

		// Visit the index expression
		const auto index = VISIT_EXPR(ctx->index);
		const auto itype = index->type;
		if (!itype->isNumericType() || itype->isMatrix() || (index->arraySize != 1)) {
			ERROR(ctx->index, "Indexer argument must by a non-array numeric scalar or vector");
		}
		if (itype->isFloat()) {
			ERROR(ctx->index, "Indexer argument must be an integer type");
		}

		// A few different types can be used as arrays
		string refStr{};
		const ShaderType* refType{};
		if (left->arraySize != 1) {
			refStr = mkstr("%s[%s]", left->refString, index->refString);
			refType = ltype;
		}
		else if (ltype->isImage()) {
			const auto dimcount = TexelRankGetComponentCount(ltype->texel.rank);
			if (dimcount != itype->numeric.dims[0]) {
				ERROR(ctx->index, mkstr("Image type expects indexer with %u components", dimcount));
			}
			refStr = mkstr("imageStore(%s, %s, {})", left->refString.c_str(), index->refString.c_str());
			refType = ltype->texel.format->asDataType();
		}
		else if (ltype->baseType == BaseType::RWBuffer) {
			if (!itype->isScalar()) {
				ERROR(ctx->index, "RWBuffer expects a scalar integer indexer");
			}
			refStr = mkstr("(%s._data_[%s])", left->refString.c_str(), ctx->index->getText().c_str());
			refType = shader_->types().getType(ltype->buffer.structType->userStruct.type->name());
		}
		else if (ltype->baseType == BaseType::RWTexels) {
			if (!itype->isScalar()) {
				ERROR(ctx->index, "RWTexels expects a scalar integer indexer");
			}
			refStr = mkstr("imageStore(%s, %s, {})", left->refString.c_str(), index->refString.c_str());
			refType = ltype->texel.format->asDataType();
		}
		else if (ltype->isNumericType()) {
			if (ltype->isMatrix()) { // Matrix
				refStr = left->refString;
				refType = TypeList::GetNumericType(ltype->baseType, ltype->numeric.size, ltype->numeric.dims[0], 1);
			}
			else if (ltype->isVector()) { // Vector
				refStr = left->refString;
				refType = TypeList::GetNumericType(ltype->baseType, ltype->numeric.size, 1, 1);
			}
			else {
				ERROR(ctx->index, "Cannot apply indexer to scalar type");
			}
		}
		else {
			ERROR(ctx->index, "Type cannot receive an indexer");
		}

		return MAKE_EXPR(refStr, refType, 1);
	}
	else { // value.member
		const auto ident = ctx->IDENTIFIER()->getText();

		// Get the lvalue
		const auto left = VISIT_EXPR(ctx->val);
		const auto ltype = left->type;
		if (left->refString.find("imageStore") == 0) {
			ERROR(ctx->val, "Image or RWTexel stores must be top-level lvalue");
		}

		// Switch on type (struct=member, vector=swizzle)
		if (ltype->isStruct()) {
			// Validate member
			const auto memType = ltype->userStruct.type->getMember(ident);
			if (!memType) {
				ERROR(ctx->IDENTIFIER(), mkstr("Type '%s' does not have member '%s'",
						ltype->userStruct.type->name().c_str(), ident.c_str()));
			}

			// Return member
			const auto refStr = mkstr("(%s.%s)", left->refString.c_str(), ident.c_str());
			const auto refType = TypeList::GetNumericType(memType->type->baseType, memType->type->numeric.size,
				memType->type->numeric.dims[0], memType->type->numeric.dims[1]);
			return MAKE_EXPR(refStr, refType, memType->arraySize);
		}
		else if (ltype->isVector()) {
			// Validate data type
			const uint32 compCount = ltype->numeric.dims[0];
			if (!ltype->isNumericType() && !ltype->isBoolean()) {
				ERROR(ctx->IDENTIFIER(), "Swizzles can only be applied to numeric types");
			}
			if ((compCount == 1) || (ltype->numeric.dims[1] != 1)) {
				ERROR(ctx->IDENTIFIER(), "Swizzles can only be applied to a vector type");
			}
			validateSwizzle(compCount, ctx->IDENTIFIER());

			// Get the new type
			const auto stype = 
				TypeList::GetNumericType(ltype->baseType, ltype->numeric.size, uint32(ident.length()), 1);
			return MAKE_EXPR(mkstr("(%s.%s)", left->refString.c_str(), ident.c_str()), stype, 1);
		}
		else {
			ERROR(ctx->val, "Operator '.' can only be applied to structs or vectors");
		}
	}

	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(IfStatement)
{
	// Check the condition
	const auto cond = VISIT_EXPR(ctx->cond);
	if (cond->arraySize != 1) {
		ERROR(ctx->cond, "If statement condition cannot be an array");
	}
	if (!cond->type->isScalar() || !cond->type->isBoolean()) {
		ERROR(ctx->cond, "If statement condition must be a scalar boolean");
	}

	// Emit and create scope
	funcGen_->emitIf(cond->refString);
	scopes_.pushScope(Scope::Conditional);

	// Visit statements
	if (ctx->statement()) {
		visit(ctx->statement());
	}
	else {
		for (const auto& stmt : ctx->statementBlock()->statement()) {
			visit(stmt);
		}
	}

	// Close scope
	funcGen_->closeBlock();
	scopes_.popScope();

	// Visit the elif and else statements
	for (const auto& elif : ctx->elifStatement()) {
		visit(elif);
	}
	if (ctx->elseStatement()) {
		visit(ctx->elseStatement());
	}

	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(ElifStatement)
{
	// Check the condition
	const auto cond = VISIT_EXPR(ctx->cond);
	if (cond->arraySize != 1) {
		ERROR(ctx->cond, "Elif statement condition cannot be an array");
	}
	if (!cond->type->isScalar() || !cond->type->isBoolean()) {
		ERROR(ctx->cond, "Elif statement condition must be a scalar boolean");
	}

	// Emit and create scope
	funcGen_->emitElif(cond->refString);
	scopes_.pushScope(Scope::Conditional);

	// Visit statements
	if (ctx->statement()) {
		visit(ctx->statement());
	}
	else {
		for (const auto& stmt : ctx->statementBlock()->statement()) {
			visit(stmt);
		}
	}

	// Close scope
	funcGen_->closeBlock();
	scopes_.popScope();

	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(ElseStatement)
{
	// Emit and create scope
	funcGen_->emitElse();
	scopes_.pushScope(Scope::Conditional);

	// Visit statements
	if (ctx->statement()) {
		visit(ctx->statement());
	}
	else {
		for (const auto& stmt : ctx->statementBlock()->statement()) {
			visit(stmt);
		}
	}

	// Close scope
	funcGen_->closeBlock();
	scopes_.popScope();

	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(ForLoopStatement)
{
	// Check the variable
	const auto counterName = ctx->counter->getText();
	validateName(ctx->counter);
	if (scopes_.hasName(counterName) || scopes_.hasGlobalName(counterName)) {
		ERROR(ctx->counter, mkstr("The name '%s' already exists and cannot be reused", counterName.c_str()));
	}

	// Check the start variable
	const auto slit = parseLiteral(ctx->start);
	if (slit.type == Literal::Float) {
		ERROR(ctx->start, "Loop start value must be an integer type");
	}
	if ((slit.i < INT32_MIN) || (slit.i > INT32_MAX)) {
		ERROR(ctx->start, "Loop end value is out of range");
	}
	const auto startValue = int32(slit.i);

	// Check the end variable
	const auto elit = parseLiteral(ctx->end);
	if (elit.type == Literal::Float) {
		ERROR(ctx->end, "Loop end value must be an integer type");
	}
	if ((elit.i < INT32_MIN) || (elit.i > INT32_MAX)) {
		ERROR(ctx->end, "Loop end value is out of range");
	}
	const auto endValue = int32(elit.i);

	// Check the optional step value
	int32 stepValue{ 1 };
	if (ctx->step) {
		// Parse
		const auto lit = parseLiteral(ctx->step);
		if (lit.type == Literal::Float) {
			ERROR(ctx->step, "Loop step value must be an integer type");
		}
		if ((lit.i < INT32_MIN) || (lit.i > INT32_MAX)) {
			ERROR(ctx->step, "Loop step value is out of range");
		}
		stepValue = int32(lit.i);

		// Validate
		if (stepValue == 0) {
			ERROR(ctx->step, "Loop step value cannot be zero");
		}
		if (signum(stepValue) != signum(endValue - startValue)) {
			ERROR(ctx->step, "Sign of step is invalid for given start and end values");
		}
	}
	else {
		if (endValue < startValue) {
			stepValue = -1;
		}
	}

	// Emit and push scope, add counter as readonly variable
	funcGen_->emitForLoop(counterName, startValue, endValue, stepValue);
	scopes_.pushScope(Scope::Loop);
	Variable counterVar{ counterName, VariableType::Private, TypeList::GetBuiltinType("int"), 1, Variable::READONLY };
	scopes_.addVariable(counterVar);

	// Visit the inner statements
	for (const auto& stmt : ctx->statementBlock()->statement()) {
		visit(stmt);
	}

	// Emit and pop scope
	funcGen_->closeBlock();
	scopes_.popScope();

	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(ControlStatement)
{
	const auto keyword = ctx->getText();

	if ((keyword == "break") || (keyword == "continue")) {
		if (!scopes_.inLoop()) {
			ERROR(ctx, mkstr("Statement '%s' only allowed in loops", keyword.c_str()));
		}
	}
	else if (keyword == "discard") {
		if (currentStage_ != ShaderStages::Fragment) {
			ERROR(ctx, "Statement 'discard' only allowed in fragment stage");
		}
	}

	funcGen_->emitControlStatement(keyword);

	return nullptr;
}

} // namespace vsl
