/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./Parser.hpp"
#include "./Expr.hpp"
#include "./Op.hpp"

#define VISIT_FUNC(type) antlrcpp::Any Parser::visit##type(grammar::VSL::type##Context* ctx)
#define MAKE_EXPR(name,type,arrSize) (std::make_shared<Expr>(name,type,arrSize))
#define VISIT_EXPR(context) (visit(context).as<std::shared_ptr<Expr>>())


namespace vsl
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
		ERROR(varDecl->baseType, "Function-local variable must be numeric or boolean types");
	}

	// Visit and check expression
	const auto expr = VISIT_EXPR(ctx->value);
	const auto etype = expr->type();
	if (!etype->hasImplicitCast(var.dataType)) {
		ERROR(ctx->value, mkstr("No implicit cast from rvalue '%s' to lvalue '%s'", etype->getVSLName().c_str(),
				var.dataType->getVSLName().c_str()));
	}

	// Add the variable
	var.type = VariableType::Private;
	scopes_.addVariable(var);

	// Emit declaration and assignment
	const auto typeStr = NameHelper::GetGeneralTypeName(var.dataType);
	generator_.emitAssignment(typeStr + " " + var.name, "=", expr->refString());

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
		ERROR(ctx->baseType, "Function-local variables must be numeric or boolean type");
	}

	// Add variable
	var.type = VariableType::Private;
	scopes_.addVariable(var);

	// Emit declaration
	generator_.emitDeclaration(var);

	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(Assignment)
{
	// Visit the left-hand side
	const auto left = VISIT_EXPR(ctx->lval);
	const auto ltype = left->type();
	const auto isImageStore = (left->refString().find("imageStore") == 0);

	// Visit expression
	const auto expr = VISIT_EXPR(ctx->value);
	const auto etype = expr->type();

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
		const string prefix =
			(etype->baseType == ShaderBaseType::Signed) ? "i" :
			(etype->baseType == ShaderBaseType::Unsigned) ? "u" : "";
		const auto valstr =
			(dims == 1) ? mkstr("%svec4(%s, 0, 0, 0)", prefix.c_str(), expr->refString().c_str()) :
			(dims == 2) ? mkstr("%svec4(%s, 0, 0)", prefix.c_str(), expr->refString().c_str()) : expr->refString();

		// Emit image store
		generator_.emitImageStore(left->refString(), valstr);
	}
	else if (!isCompound) {
		if (!etype->hasImplicitCast(left->type())) {
			ERROR(ctx->value, mkstr("No implicit cast from rvalue '%s' to lvalue '%s'", etype->getVSLName().c_str(),
				left->type()->getVSLName().c_str()));
		}
		// Emit assignment
		generator_.emitAssignment(left->refString(), optxt, expr->refString());
	}
	else {
		const auto subop = optxt.substr(0, optxt.length() - 1);

		// Process expressions
		const ShaderType* restype{};
		const auto refstr = Op::ProcessBinaryOp(subop, left.get(), expr.get(), &restype);
		if (refstr.empty()) {
			ERROR(ctx->value, mkstr("Compound assignment '%s' not possible with types '%s' and '%s'",
				optxt.c_str(), ltype->getVSLName().c_str(), etype->getVSLName().c_str()));
		}

		// Emit assignment
		generator_.emitAssignment(left->refString(), optxt, expr->refString());
	}

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

		// Get the name of the variable based on the variable type
		string outname{};
		switch (var->type)
		{
		case VariableType::Binding: {
			if (var->dataType->baseType == ShaderBaseType::Uniform) {
				outname = var->name;
			}
			else if (var->dataType->isBuffer()) {
				generator_.emitBindingIndex(var->extra.binding.slot);
				outname = mkstr("%s[_bidx%u_]", var->name.c_str(), uint32(var->extra.binding.slot));
			}
			else {
				const auto table = NameHelper::GetBindingTableName(var->dataType);
				generator_.emitBindingIndex(var->extra.binding.slot);
				outname = mkstr("(%s[_bidx%u_])", table.c_str(), uint32(var->extra.binding.slot));
			}
		} break;
		case VariableType::Builtin: {
			outname = NameHelper::GetBuiltinName(var->name);
		} break;
		case VariableType::Local: {
			outname = mkstr("_%s_%s", ShaderStageToStr(currentStage_).c_str(), varName.c_str());
		} break;
		default: outname = varName;
		}

		return MAKE_EXPR(outname, var->dataType, var->arraySize);
	}
	else {
		// Get the lvalue
		const auto left = VISIT_EXPR(ctx->val);
		const auto ltype = left->type();
		if (left->refString().find("imageStore") == 0) {
			ERROR(ctx->val, "Image or RWTexel stores must be top-level lvalue");
		}
		
		// Switch based on the lvalue type
		if (ctx->index) {
			// Visit the index expression
			const auto index = VISIT_EXPR(ctx->index);
			const auto itype = index->type();
			if (!itype->isNumeric() || itype->isMatrix() || (index->arraySize() != 1)) {
				ERROR(ctx->index, "Indexer argument must by a non-array numeric scalar or vector");
			}
			if (itype->isFloat()) {
				ERROR(ctx->index, "Indexer argument must be an integer type");
			}

			// A few different types can be used as arrays
			string refStr{};
			const ShaderType* refType{};
			if (ltype->isImage()) {
				const auto dimcount = GetImageDimsComponentCount(ltype->image.dims);
				if (dimcount != itype->numeric.dims[0]) {
					ERROR(ctx->index, mkstr("Image type expects indexer with %u components", dimcount));
				}
				refStr = mkstr("imageStore(%s, %s, {})", left->refString().c_str(), index->refString().c_str());
				refType = 
					TypeManager::GetNumericType(ltype->image.texel.type, ltype->image.texel.components, 1);
			}
			else if (ltype->baseType == ShaderBaseType::RWBuffer) {
				if (!itype->isScalar()) {
					ERROR(ctx->index, "RWBuffer expects a scalar integer indexer");
				}
				refStr = mkstr("(%s._data_[%s])", left->refString().c_str(), ctx->index->getText().c_str());
				refType = types_.getType(ltype->buffer.structType->userStruct.structName);
			}
			else if (ltype->baseType == ShaderBaseType::RWTexels) {
				if (!itype->isScalar()) {
					ERROR(ctx->index, "RWTexels expects a scalar integer indexer");
				}
				refStr = mkstr("imageStore(%s, %s, {})", left->refString().c_str(), index->refString().c_str());
				refType =
					TypeManager::GetNumericType(ltype->image.texel.type, ltype->image.texel.components, 1);
			}
			else if (left->arraySize() != 1) {
				refStr = left->refString();
				refType = ltype;
			}
			else if (ltype->isNumeric()) {
				if (ltype->isMatrix()) { // Matrix
					refStr = left->refString();
					refType = TypeManager::GetNumericType(ltype->baseType, ltype->numeric.dims[0], 1);
				}
				else if (ltype->isVector()) { // Vector
					refStr = left->refString();
					refType = TypeManager::GetNumericType(ltype->baseType, 1, 1);
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
		else { // ctx->IDENTIFIER()
			const auto ident = ctx->IDENTIFIER()->getText();

			// Switch on type (struct=member, vector=swizzle)
			if (ltype->isStruct()) {
				// Validate member
				const auto memType = ltype->getMember(ident);
				if (!memType) {
					ERROR(ctx->IDENTIFIER(),
						mkstr("Struct type '%s' does not have member with name '%s'",
							ltype->userStruct.structName.c_str(), ident.c_str()));
				}

				// Return member
				const auto refStr = mkstr("(%s.%s)", left->refString().c_str(), ident.c_str());
				const auto refType = TypeManager::GetNumericType(memType->baseType, memType->dims[0], memType->dims[1]);
				return MAKE_EXPR(refStr, refType, memType->arraySize);
			}
			else if (ltype->isVector()) {
				// Validate data type
				const uint32 compCount = ltype->numeric.dims[0];
				if (!ltype->isNumeric() && (ltype->baseType != ShaderBaseType::Boolean)) {
					ERROR(ctx->IDENTIFIER(), "Swizzles can only be applied to numeric types");
				}
				if ((compCount == 1) || (ltype->numeric.dims[1] != 1)) {
					ERROR(ctx->IDENTIFIER(), "Swizzles can only be applied to a vector type");
				}
				validateSwizzle(compCount, ctx->IDENTIFIER());

				// Get the new type
				const auto stype = TypeManager::GetNumericType(ltype->baseType, uint32(ident.length()), 1);
				return MAKE_EXPR(mkstr("(%s.%s)", left->refString().c_str(), ident.c_str()), stype, 1);
			}
			else {
				ERROR(ctx->val, "Operator '.' can only be applied to structs (members) or vectors (swizzles)");
			}
		}
	}
}

} // namespace vsl
