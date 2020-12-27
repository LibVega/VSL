/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./Parser.hpp"
#include "./Expr.hpp"

#define VISIT_FUNC(type) antlrcpp::Any Parser::visit##type(grammar::PLSL::type##Context* ctx)


namespace plsl
{

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
	if (!var.dataType->isNumeric() && (var.dataType->baseType != ShaderBaseType::Boolean)) {
		ERROR(varDecl->type, "Function-local variable must be numeric or boolean types");
	}

	// TODO: Visit and check expression

	// Add the variable
	var.type = VariableType::Private;
	scopes_.addVariable(var);

	// TODO: Emit declaration and assignment

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
	if (!var.dataType->isNumeric() && (var.dataType->baseType != ShaderBaseType::Boolean)) {
		ERROR(ctx->type, "Function-local variables must be numeric or boolean type");
	}

	// Add variable
	var.type = VariableType::Private;
	scopes_.addVariable(var);

	// TODO: Emit declaration

	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(Assignment)
{
	// Visit the left-hand side (abuse the Variable type to accomplish this)
	const std::shared_ptr<Variable> var{ visit(ctx->lval).as<Variable*>() };

	// TODO: Visit expression

	// TODO: Validate type compatiblity and compound-assignment operators

	// TODO: Emit assignment

	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(Lvalue)
{
	// Process different lvalue options
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

		return new Variable({}, varName, var->dataType, var->arraySize);
	}
	else {
		// Get the lvalue
		const std::shared_ptr<Variable> var{ visit(ctx->val).as<Variable*>() };
		
		// Switch based on the lvalue type
		if (ctx->index) {
			// TODO: Visit the expression
			string index{ "TODO" };

			// A few different types can be used as arrays
			string refName{};
			const ShaderType* refType{};
			if (var->dataType->isBuffer()) { // Would only be RWBuffer at this point
				refName = mkstr("%s._data_", var->name.c_str());
				refType = types_.getType(var->dataType->buffer.structName);
			}
			else if (var->arraySize != 1) {
				refName = var->name;
				refType = var->dataType;
			}
			else if (var->dataType->isNumeric()) {
				if (var->dataType->numeric.dims[1] != 1) { // Matrix
					refName = var->name;
					refType = types_.getNumericType(var->dataType->baseType, var->dataType->numeric.dims[0], 1);
				}
				else if (var->dataType->numeric.dims[0] != 1) { // Vector
					refName = var->name;
					refType = types_.getNumericType(var->dataType->baseType, 1, 1);
				}
				else {
					ERROR(ctx->index, "Cannot apply array index to scalar type");
				}
			}
			else {
				ERROR(ctx->index, "Type cannot receive an array index");
			}

			return new Variable({}, mkstr("(%s[%s])", refName.c_str(), index.c_str()), refType, 1);
		}
		else if (ctx->SWIZZLE()) {
			// Validate data type
			const uint32 compCount = var->dataType->numeric.dims[0];
			if (!var->dataType->isNumeric() && (var->dataType->baseType != ShaderBaseType::Boolean)) {
				ERROR(ctx->SWIZZLE(), "Swizzles can only be applied to numeric types");
			}
			if ((compCount == 1) || (var->dataType->numeric.dims[1] != 1)) {
				ERROR(ctx->SWIZZLE(), "Swizzles can only be applied to a vector type");
			}
			validateSwizzle(compCount, ctx->SWIZZLE());

			// Get the new type
			const auto stxt = ctx->SWIZZLE()->getText();
			const auto stype = types_.getNumericType(var->dataType->baseType, uint32(stxt.length()), 1);

			return new Variable({}, mkstr("(%s.%s)", var->name.c_str(), stxt.c_str()), stype, 1);
		}
		else { // ctx->IDENTIFIER()
			// Validate data type
			if (!var->dataType->isStruct()) {
				ERROR(ctx->IDENTIFIER(), "Member access operator '.' can only be applied to struct types");
			}

			// Validate member
			const auto memName = ctx->IDENTIFIER()->getText();
			const auto memType = var->dataType->getMember(memName);
			if (!memType) {
				ERROR(ctx->IDENTIFIER(), 
					mkstr("Struct type '%s' does not have member with name '%s'", 
						var->dataType->userStruct.structName.c_str(), memName.c_str()));
			}

			return new Variable({}, mkstr("(%s.%s)", var->name.c_str(), memName.c_str()), 
				types_.getNumericType(memType->baseType, memType->dims[0], memType->dims[1]), memType->arraySize);
		}
	}
}

} // namespace plsl
