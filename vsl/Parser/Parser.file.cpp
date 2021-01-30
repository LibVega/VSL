/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./Parser.hpp"

#define VISIT_FUNC(type) antlrcpp::Any Parser::visit##type(grammar::VSL::type##Context* ctx)


namespace vsl
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
VISIT_FUNC(ShaderStructDefinition)
{
	// Check parse status
	if (bool(shader_->info().stageMask())) {
		ERROR(ctx, "All user-defined types must be provided before the first stage function");
	}

	// Get type name and check
	const auto typeName = ctx->name->getText();
	if (shader_->types().getType(typeName)) {
		ERROR(ctx->name, mkstr("Duplicate type name '%s'", typeName.c_str()));
	}
	if (typeName.length() > Shader::MAX_NAME_LENGTH) {
		ERROR(ctx->name, mkstr("Type names cannot be longer than %u characters", Shader::MAX_NAME_LENGTH));
	}
	if (typeName[0] == '_' && *(typeName.rbegin()) == '_') {
		ERROR(ctx->name, "Type names that start and end with '_' are reserved");
	}

	// Parse the field declarations
	std::vector<StructType::Member> members{};
	std::vector<string> names{};
	for (const auto field : ctx->variableDeclaration()) {
		// Create variable
		const auto fVar = parseVariableDeclaration(field, true);

		// Check name
		if (std::find(names.begin(), names.end(), fVar.name) != names.end()) {
			ERROR(field->name, mkstr("Duplicate struct field '%s'", fVar.name.c_str()));
		}

		// Check field type
		if (!fVar.dataType->isNumericType() && !fVar.dataType->isBoolean()) {
			ERROR(field->baseType, mkstr("Struct field '%s' must be numeric", fVar.name.c_str()));
		}

		// Add the member
		StructType::Member member{};
		member.name = fVar.name;
		member.arraySize = fVar.arraySize;
		member.type = fVar.dataType;
		members.push_back(member);
		names.push_back(fVar.name);
	}
	if (members.size() == 0) {
		ERROR(ctx->name, "Empty struct types are not allowed");
	}
	StructType structType{ typeName, members };
	if (structType.size() > Shader::MAX_STRUCT_SIZE) {
		ERROR(ctx->name, mkstr("Struct types cannot be larger than %u bytes", Shader::MAX_STRUCT_SIZE));
	}

	// Add the struct type
	const auto sType = shader_->types().addStructType(typeName, structType);
	shader_->types().addType(typeName, { sType });

	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(ShaderInputOutputStatement)
{
	// Check parse status
	if (bool(shader_->info().stageMask())) {
		ERROR(ctx, "All interface variables must be provided before the first stage function");
	}

	// Get direction and interface index
	const bool isIn = ctx->io->getText() == "in";
	const auto indexLiteral = parseLiteral(ctx->index);
	if (indexLiteral.isNegative()) {
		ERROR(ctx->index, "Negative binding index not allowed");
	}
	const auto index = uint32(indexLiteral.u);

	// Validate index
	if (isIn) {
		if (index >= Shader::MAX_VERTEX_ATTRIBS) {
			ERROR(ctx->index, mkstr("Vertex input is higher than max binding %u", Shader::MAX_VERTEX_ATTRIBS - 1));
		}
		const auto other = shader_->info().getInput(index);
		if (other) {
			ERROR(ctx->index, (other->location == index)
				? mkstr("Vertex input %u is already populated by '%s'", index, other->name.c_str())
				: mkstr("Vertex input %u overlaps with input '%s'", index, other->name.c_str()));
		}
	}
	else {
		if (index >= Shader::MAX_FRAGMENT_OUTPUTS) {
			ERROR(ctx->index, mkstr("Fragment output is higher than max binding %u", Shader::MAX_FRAGMENT_OUTPUTS - 1));
		}
		const auto other = shader_->info().getOutput(index);
		if (other) {
			ERROR(ctx->index, mkstr("Fragment output %u is already populated by '%s'", index, other->name.c_str()));
		}
	}

	// Parse decl
	const auto varDecl = ctx->variableDeclaration();
	auto ioVar = parseVariableDeclaration(varDecl, true);

	// Input/Output specific type validation
	if (isIn) {
		ioVar.varType = VariableType::Input;
		if (ioVar.arraySize > Shader::MAX_VERTEX_ATTRIBS) {
			ERROR(varDecl->arraySize, mkstr("Vertex arrays cannot be larger than %u", Shader::MAX_VERTEX_ATTRIBS));
		}
		if ((ioVar.arraySize != 1) && ioVar.dataType->isMatrix()) {
			ERROR(varDecl->arraySize, "Vertex inputs that are matrix types cannot be arrays");
		}
	}
	else { // Outputs must be non-arrays, and either scalars or vectors
		ioVar.varType = VariableType::Output;
		if (ioVar.arraySize != 1) {
			ERROR(varDecl->arraySize, "Fragment outputs cannot be arrays");
		}
		if (ioVar.dataType->isMatrix()) {
			ERROR(varDecl->baseType, "Fragment outputs cannot be matrix types");
		}
		if (ioVar.dataType->numeric.dims[0] == 3) {
			ERROR(varDecl->baseType, "Fragment outputs cannot be 3-component vectors");
		}
	}

	// Add to shader info
	InterfaceVariable infovar{ varDecl->name->getText(), index, ioVar.dataType, ioVar.arraySize };
	if (isIn) {
		shader_->info().inputs().push_back(infovar);
	}
	else {
		shader_->info().outputs().push_back(infovar);
	}
	scopes_.addGlobal(ioVar);

	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(ShaderUniformStatement)
{
	// Check parse status
	if (bool(shader_->info().stageMask())) {
		ERROR(ctx, "Uniform must be provided before the first stage function");
	}
	if (shader_->info().hasUniform()) {
		ERROR(ctx, "A shader can only have one uniform declaration");
	}

	// Parse the variable declaration
	const auto varDecl = ctx->variableDeclaration();
	const auto bVar = parseVariableDeclaration(varDecl, true);
	if (!bVar.dataType->isStruct()) {
		ERROR(varDecl->baseType, "Uniforms must be structs");
	}
	if (bVar.arraySize != 1) {
		ERROR(varDecl->arraySize, "Uniforms cannot be arrays");
	}

	// Create the new uniform type
	ShaderType tempType{ BaseType::Uniform, bVar.dataType };
	const auto uType = 
		shader_->types().addType(mkstr("Uniform<%s>", bVar.dataType->userStruct.type->name().c_str()), tempType);

	// Add to the shader info and scope
	shader_->info().uniform({ bVar.name, uType, 0 });
	scopes_.addGlobal({ bVar.name, VariableType::Binding, uType, 1, Variable::READONLY });

	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(ShaderBindingStatement)
{
	// Check parse status
	if (bool(shader_->info().stageMask())) {
		ERROR(ctx, "All bindings must be provided before the first stage function");
	}

	// Check for binding limit
	if (shader_->info().bindings().size() == Shader::MAX_BINDINGS) {
		ERROR(ctx, mkstr("Cannot have more than %u bindings in a shader", Shader::MAX_BINDINGS));
	}

	// Parse the variable declaration
	const auto varDecl = ctx->variableDeclaration();
	const auto bVar = parseVariableDeclaration(varDecl, true);
	if (bVar.dataType->isNumericType() || bVar.dataType->isBoolean() || bVar.dataType->isStruct()) {
		ERROR(varDecl->baseType, "Bindings cannot be numeric, boolean, or struct types");
	}
	if (bVar.arraySize != 1) {
		ERROR(varDecl->arraySize, "Bindings cannot be arrays");
	}

	// Get the binding slot
	const auto slotLiteral = parseLiteral(ctx->slot);
	if (slotLiteral.isNegative() || (slotLiteral.type == Literal::Float)) {
		ERROR(ctx->slot, "Binding slot index must be non-negative integer");
	}
	else if (slotLiteral.u >= Shader::MAX_BINDINGS) {
		ERROR(ctx->slot, mkstr("Slot index out of range (max %u)", Shader::MAX_BINDINGS - 1));
	}
	const auto slotIndex = uint32(slotLiteral.u);
	const auto existing = shader_->info().getBinding(slotIndex);
	if (existing) {
		ERROR(ctx->slot, mkstr("Binding slot %u is already populated by '%s'", slotIndex, existing->name.c_str()));
	}

	// Add to info and scope
	shader_->info().bindings().push_back({ bVar.name, bVar.dataType, slotIndex });
	const auto canWrite =
		(bVar.dataType->isImage() || bVar.dataType->isRWBuffer() || bVar.dataType->isRWTexels());
	Variable var {
		bVar.name, VariableType::Binding, bVar.dataType, 1, canWrite ? Variable::READWRITE : Variable::READONLY
	};
	var.extra.binding.slot = slotIndex;
	scopes_.addGlobal(var);

	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(ShaderLocalStatement)
{
	// Check parse status
	if (bool(shader_->info().stageMask())) {
		ERROR(ctx, "All locals must be provided before the first stage function");
	}

	// Parse and validate variable
	const auto isFlat = !!ctx->KW_FLAT();
	const auto varDecl = ctx->variableDeclaration();
	const auto lVar = parseVariableDeclaration(varDecl, true);
	if (lVar.arraySize != 1) {
		ERROR(varDecl->arraySize, "Shader locals cannot be arrays");
	}
	if (!lVar.dataType->isNumericType() || lVar.dataType->isMatrix()) {
		ERROR(varDecl->baseType, "Shader locals must be numeric scalars or vectors");
	}
	if (lVar.dataType->isInteger() && !isFlat) {
		ERROR(varDecl->baseType, "Shader locals with integer types must be declared as 'flat'");
	}

	// Parse and validate stages
	const auto pStage = StrToShaderStage(ctx->pstage->getText());
	if (pStage == ShaderStages::None) {
		ERROR(ctx->pstage, mkstr("Unknown shader stage '%s'", ctx->pstage->getText().c_str()));
	}
	if ((pStage == ShaderStages::TessControl) || (pStage == ShaderStages::TessEval) ||
		(pStage == ShaderStages::Geometry)) {
		ERROR(ctx->pstage, "Currently only vertex stages can produce locals");
	}

	// Add variable
	Variable var{ lVar.name, VariableType::Local, lVar.dataType, 1, Variable::READWRITE };
	var.extra.local.sourceStage = pStage;
	var.extra.local.flat = isFlat;
	scopes_.addGlobal(var);

	// Add to info
	shader_->info().locals().push_back({ lVar.name, pStage, lVar.dataType, isFlat });

	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(ShaderSubpassInputStatement)
{
	// Check parse status
	if (bool(shader_->info().stageMask())) {
		ERROR(ctx, "All subpass inputs must be provided before the first stage function");
	}

	// Parse and validate format
	const auto fmtText = ctx->format->getText();
	const auto format = TypeList::GetTexelFormat(fmtText);
	if (!format) {
		ERROR(ctx->format, mkstr("No texel format '%s' found", fmtText.c_str()));
	}
	if (format->isNormlizedType() || (format->count != 4) || (format->size != 4)) {
		ERROR(ctx->format, "Only int4, uint4, and float4 allowed for subpass inputs");
	}

	// Validate name
	validateName(ctx->name);

	// Get the binding slot
	const auto indexLiteral = parseLiteral(ctx->index);
	if (indexLiteral.isNegative() || (indexLiteral.type == Literal::Float)) {
		ERROR(ctx->index, "Subpass input index must be non-negative integer");
	}
	else if (indexLiteral.u >= Shader::MAX_SUBPASS_INPUTS) {
		ERROR(ctx->index, mkstr("Slot index out of range (max %u)", Shader::MAX_SUBPASS_INPUTS - 1));
	}
	const auto slotIndex = uint32(indexLiteral.u);
	const auto existing = shader_->info().getSubpassInput(slotIndex);
	if (existing) {
		ERROR(ctx->index, mkstr("Subpass input %u is already populated by '%s'", slotIndex, existing->name.c_str()));
	}
	if ((slotIndex != 0) && !shader_->info().getSubpassInput(slotIndex - 1)) {
		ERROR(ctx->index, "Subpass inputs must have contiguous indices");
	}

	// Add the type
	const auto typeName = mkstr("Spi<%s>", format->getVSLName());
	auto spiType = shader_->types().getType(typeName);
	if (!spiType) {
		spiType = shader_->types().addType(typeName, { BaseType::SPInput, TexelRank::E2D, format });
	}

	// Add to info and scope
	shader_->info().subpassInputs().push_back({ ctx->name->getText(), slotIndex, format });
	Variable var{ ctx->name->getText(), VariableType::Binding, spiType, 1, Variable::READONLY };
	var.extra.binding.slot = slotIndex;
	scopes_.addGlobal(var);

	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(ShaderStageFunction)
{
	// Validate stage
	const auto stageName = ctx->stage->getText();
	const auto stage = StrToShaderStage(stageName);
	if (stage == ShaderStages::None) {
		ERROR(ctx->stage, mkstr("Unknown shader stage '%s' for function", stageName.c_str()));
	}
	if ((shader_->info().stageMask() & stage) == stage) {
		ERROR(ctx->stage, mkstr("Duplicate shader function for stage '%s'", stageName.c_str()));
	}

	// No support for tesc/tese/geom yet
	if (stage == ShaderStages::TessControl) {
		ERROR(ctx->stage, "Tessellation control stage is not yet supported");
	}
	if (stage == ShaderStages::TessEval) {
		ERROR(ctx->stage, "Tessellation evaluation stage is not yet supported");
	}
	if (stage == ShaderStages::Geometry) {
		ERROR(ctx->stage, "Geometry stage is not yet supported");
	}

	// Push the global scope for the stage
	scopes_.pushGlobalScope(stage);
	funcGen_ = shader_->getOrCreateFunctionGenerator(stage);

	// Visit the function statements
	currentStage_ = stage;
	for (const auto stmt : ctx->statementBlock()->statement()) {
		visit(stmt);
	}
	currentStage_ = ShaderStages::None;

	// Pop the global scope
	scopes_.popScope();
	funcGen_->emitClose();
	funcGen_ = nullptr;

	// Update shader info
	shader_->info().stageMask(shader_->info().stageMask() | stage);

	return nullptr;
}

} // namespace vsl
