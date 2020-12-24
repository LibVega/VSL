/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./Parser.hpp"

#define VISIT_FUNC(type) antlrcpp::Any Parser::visit##type(grammar::PLSL::type##Context* ctx)


namespace plsl
{

// ====================================================================================================================
VISIT_FUNC(File)
{
	// Visit the shader type statement
	visit(ctx->shaderTypeStatement());

	// Visit all top level statements
	for (const auto tls : ctx->topLevelStatement()) {
		visit(tls);
	}

	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(ShaderTypeStatement)
{
	// Validate the shader type
	const auto shaderType = ctx->type->getText();
	if (shaderType == "graphics") {
		return nullptr; // Only graphics supported for now
	}
	else if (shaderType == "compute") {
		ERROR(ctx->type, "Compute shaders are not yet supported");
	}
	else if (shaderType == "ray") {
		ERROR(ctx->type, "Ray shaders are not yet supported");
	}
	else {
		ERROR(ctx->type, mkstr("Unknown shader type '%s'", shaderType.c_str()));
	}

	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(ShaderUserTypeDefinition)
{
	// Get type name and check
	const auto typeName = ctx->name->getText();
	if (types_.getType(typeName)) {
		ERROR(ctx->name, mkstr("Duplicate type name '%s'", typeName.c_str()));
	}

	// Parse the field declarations
	ShaderType structType{ typeName, {} };
	for (const auto field : ctx->variableDeclaration()) {
		// Create variable
		const auto fVar = parseVariableDeclaration(field);

		// Check name
		if (structType.hasMember(fVar.name)) {
			ERROR(field->name, mkstr("Duplicate field name '%s'", fVar.name.c_str()));
		}

		// Check field type
		if (!fVar.dataType->isNumeric()) {
			ERROR(field->type, mkstr("Invalid field '%s' - type fields must be numeric", fVar.name.c_str()));
		}

		// Add the member
		StructMember mem{};
		mem.name = fVar.name;
		mem.baseType = fVar.dataType->baseType;
		mem.size = fVar.dataType->numeric.size;
		mem.dims[0] = fVar.dataType->numeric.dims[0];
		mem.dims[1] = fVar.dataType->numeric.dims[1];
		mem.arraySize = fVar.arraySize;
		structType.userStruct.members.push_back(mem);
	}

	// Add the struct type to the type manager
	types_.addType(structType.userStruct.structName, structType);

	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(ShaderInputOutputStatement)
{
	// Get direction and interface index
	const bool isIn = ctx->io->getText() == "in";
	const auto indexLiteral = ParseLiteral(this, ctx->index);
	if (indexLiteral.isNegative()) {
		ERROR(ctx->index, "Negative binding index not allowed");
	}
	const auto index = uint32(indexLiteral.u);

	// Validate index
	if (isIn) {
		if (index > PLSL_MAX_INPUT_INDEX) {
			ERROR(ctx->index, mkstr("Vertex input is larger than max allowed binding %u", PLSL_MAX_INPUT_INDEX));
		}
		const auto other = shaderInfo_.getInput(index);
		if (other) {
			ERROR(ctx->index, (other->location == index)
				? mkstr("Vertex input slot '%u' is already filled by '%s'", index, other->name.c_str())
				: mkstr("Vertex input slot '%u' overlaps with input '%s'", index, other->name.c_str()));
		}
	}
	else {
		if (index > PLSL_MAX_OUTPUT_INDEX) {
			ERROR(ctx->index, mkstr("Fragment output is larger than max allowed binding %u", PLSL_MAX_OUTPUT_INDEX));
		}
		const auto other = shaderInfo_.getOutput(index);
		if (other) {
			ERROR(ctx->index, mkstr("Fragment output slot '%s' is already filled by '%s'", index, other->name.c_str()));
		}
		if ((index != 0) && !shaderInfo_.getOutput(index - 1)) {
			ERROR(ctx->index, "Fragment output indices must be contiguous");
		}
	}

	// Get and validate the type
	const auto varDecl = ctx->variableDeclaration();
	auto ioVar = parseVariableDeclaration(varDecl);
	if (!ioVar.dataType->isNumeric()) {
		ERROR(varDecl->type, "Shader interface variables must be a numeric type");
	}

	// Input/Output specific type validation
	if (isIn) {
		if (ioVar.arraySize > PLSL_MAX_INPUT_ARRAY_SIZE) {
			ERROR(varDecl->arraySize, mkstr("Vertex input arrays cannot be larger than %u", PLSL_MAX_INPUT_ARRAY_SIZE));
		}
		if ((ioVar.arraySize != 1) && (ioVar.dataType->numeric.dims[1] != 1)) {
			ERROR(varDecl->arraySize, "Vertex inputs that are matrix types cannot be arrays");
		}
	}
	else { // Outputs must be non-arrays, and either scalars or vectors
		if (ioVar.arraySize != 1) {
			ERROR(varDecl->arraySize, "Fragment outputs cannot be arrays");
		}
		if (ioVar.dataType->numeric.dims[1] != 1) {
			ERROR(varDecl->type, "Fragment outputs cannot be matrix types");
		}
	}

	// Add to shader info
	InterfaceVariable iovar{ varDecl->name->getText(), index, *ioVar.dataType, ioVar.arraySize };
	if (isIn) {
		shaderInfo_.inputs().push_back(iovar);
		ioVar.type = VariableType::Input;
	}
	else {
		shaderInfo_.outputs().push_back(iovar);
		ioVar.type = VariableType::Output;
	}
	scopes_.addGlobal(ioVar);

	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(ShaderConstantStatement)
{
	// Validate the variable
	const auto varDecl = ctx->variableDeclaration();
	const auto cVar = parseVariableDeclaration(varDecl);
	if (varDecl->arraySize) {
		ERROR(varDecl->arraySize, "Constants cannot be arrays");
	}
	if (!cVar.dataType->isNumeric()) {
		ERROR(varDecl->type, "Constants must be a numeric type");
	}
	if ((cVar.dataType->numeric.dims[0] != 1) || (cVar.dataType->numeric.dims[1] != 1)) {
		ERROR(varDecl->type, "Constants must be a scalar numeric type");
	}
	if (cVar.dataType->numeric.size != 4) {
		ERROR(varDecl->type, "Constants must be a 4-byte scalar type");
	}

	// Parse the literal
	const auto valueLiteral = ParseLiteral(this, ctx->value);
	Constant cnst;
	if (cVar.dataType->baseType == ShaderBaseType::UInteger) {
		if (valueLiteral.type == Literal::Float) {
			ERROR(ctx->value, "Cannot initialize integer constant with float literal");
		}
		if (valueLiteral.isNegative()) {
			ERROR(ctx->value, "Cannot initialized unsigned constant with negative value");
		}
		if (valueLiteral.u > UINT32_MAX) {
			ERROR(ctx->value, "Constant literal is out of range");
		}
		cnst = { varDecl->name->getText(), uint32(valueLiteral.u) };
	}
	else if (cVar.dataType->baseType == ShaderBaseType::SInteger) {
		if (valueLiteral.type == Literal::Float) {
			ERROR(ctx->value, "Cannot initialize integer constant with float literal");
		}
		if ((valueLiteral.i < INT32_MIN) || (valueLiteral.i > INT32_MAX)) {
			ERROR(ctx->value, "Constant literal is out of range");
		}
		cnst = { varDecl->name->getText(), int32(valueLiteral.i) };
	}
	else {
		if ((valueLiteral.type == Literal::Float) && ((valueLiteral.f < FLT_MIN) || (valueLiteral.f > FLT_MAX))) {
			ERROR(ctx->value, "Constant literal is out of range");
		}
		cnst = { varDecl->name->getText(),
			(valueLiteral.type == Literal::Unsigned) ? float(valueLiteral.u) :
			(valueLiteral.type == Literal::Signed) ? float(valueLiteral.i) :
			float(valueLiteral.f)
		};
	}

	// Add the constant
	scopes_.addConstant(cnst);

	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(ShaderBindingStatement)
{
	// Check for binding limit
	if (shaderInfo_.bindings().size() == (PLSL_MAX_BINDING_INDEX + 1)) {
		ERROR(ctx, mkstr("Cannot have more than %u bindings in a shader", PLSL_MAX_BINDING_INDEX + 1));
	}

	// Parse the variable declaration
	const auto varDecl = ctx->variableDeclaration();
	const auto bVar = parseVariableDeclaration(varDecl);
	if (bVar.dataType->isNumeric() || (bVar.dataType->baseType == ShaderBaseType::Boolean)) {
		ERROR(varDecl->type, "Bindings cannot be numeric or boolean types");
	}
	if (bVar.dataType->isStruct()) {
		ERROR(varDecl->type, "Bindings that are structs must be a buffer type");
	}

	// Get the binding slot
	const auto slot = ctx->slot->getText();
	const BindingGroup bGroup =
		(slot[0] == 't') ? BindingGroup::Texture :
		(slot[0] == 'b') ? BindingGroup::Buffer : BindingGroup::Input;
	const auto slotIndex = uint8(std::strtoul(slot.substr(1).data(), nullptr, 10));
	if (slotIndex > PLSL_MAX_BINDING_INDEX) {
		ERROR(ctx->slot, mkstr("Slot index out of range (max %u)", PLSL_MAX_BINDING_INDEX));
	}

	// Check and add to the shader info
	if (shaderInfo_.getBinding(bGroup, slotIndex)) {
		ERROR(ctx->slot, mkstr("Binding slot %s is already filled by another binding", ctx->slot->getText().c_str()));
	}
	shaderInfo_.bindings().push_back({ bVar.name, *bVar.dataType, bGroup, slotIndex });

	// Add to the scope manager
	Variable scopeVar = bVar;
	scopeVar.type = VariableType::Binding;
	scopes_.addGlobal(scopeVar);

	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(ShaderLocalStatement)
{
	// Parse and validate variable
	const auto isFlat = !!ctx->KW_FLAT();
	const auto varDecl = ctx->variableDeclaration();
	const auto lVar = parseVariableDeclaration(varDecl);
	if (lVar.arraySize != 1) {
		ERROR(varDecl->arraySize, "Shader locals cannot be arrays");
	}
	if (!lVar.dataType->isNumeric() || (lVar.dataType->numeric.dims[1] != 1)) {
		ERROR(varDecl->type, "Shader locals must be numeric scalars or vectors");
	}
	if ((lVar.dataType->baseType == ShaderBaseType::UInteger) || (lVar.dataType->baseType == ShaderBaseType::SInteger)) {
		if (!isFlat) {
			ERROR(varDecl->type, "Shader locals with integer types must be declared as 'flat'");
		}
	}

	// Parse and validate stages
	const auto pStage = StrToShaderStage(ctx->pstage->getText());
	if (pStage == ShaderStages::None) {
		ERROR(ctx->pstage, mkstr("Unknown shader stage '%s'", ctx->pstage->getText().c_str()));
	}
	const auto cStage = StrToShaderStage(ctx->cstage->getText());
	if (cStage == ShaderStages::None) {
		ERROR(ctx->cstage, mkstr("Unknown shader stage '%s'", ctx->cstage->getText().c_str()));
	}
	if (pStage >= cStage) {
		ERROR(ctx->cstage, "Local consumer stage must come after the producer stage");
	}

	// Add variable
	Variable newVar = lVar;
	newVar.type = VariableType::Local;
	newVar.extra.local.pStage = pStage;
	newVar.extra.local.cStage = cStage;
	newVar.extra.local.flat = isFlat;
	scopes_.addGlobal(newVar);

	return nullptr;
}

} // namespace plsl
