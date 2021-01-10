/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./Parser.hpp"

#include <cfloat>

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
	if (bool(shaderInfo_.stages())) {
		ERROR(ctx, "All user-defined types must be provided before the first stage function");
	}

	// Get type name and check
	const auto typeName = ctx->name->getText();
	if (types_.getType(typeName)) {
		ERROR(ctx->name, mkstr("Duplicate type name '%s'", typeName.c_str()));
	}
	if (typeName.length() > MAX_NAME_LENGTH) {
		ERROR(ctx->name, mkstr("Type names cannot be longer than %u characters", MAX_NAME_LENGTH));
	}
	if (typeName[0] == '_' && *(typeName.rbegin()) == '_') {
		ERROR(ctx->name, "Type names that start and end with '_' are reserved");
	}

	// Parse the field declarations
	ShaderType structType{ typeName, {} };
	for (const auto field : ctx->variableDeclaration()) {
		// Create variable
		const auto fVar = parseVariableDeclaration(field, true);

		// Check name
		if (structType.hasMember(fVar.name)) {
			ERROR(field->name, mkstr("Duplicate field name '%s'", fVar.name.c_str()));
		}

		// Check field type
		if (!fVar.dataType->isNumeric()) {
			ERROR(field->baseType, mkstr("Invalid field '%s' - type fields must be numeric", fVar.name.c_str()));
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

	// Emit the struct
	generator_.emitStruct(typeName, structType.userStruct.members);

	// Add the struct type to the type manager
	types_.addType(structType.userStruct.structName, structType);

	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(ShaderInputOutputStatement)
{
	// Check parse status
	if (bool(shaderInfo_.stages())) {
		ERROR(ctx, "All interface variables must be provided before the first stage function");
	}

	// Get direction and interface index
	const bool isIn = ctx->io->getText() == "in";
	const auto indexLiteral = ParseLiteral(this, ctx->index);
	if (indexLiteral.isNegative()) {
		ERROR(ctx->index, "Negative binding index not allowed");
	}
	const auto index = uint32(indexLiteral.u);

	// Validate index
	if (isIn) {
		if (index > VSL_MAX_INPUT_INDEX) {
			ERROR(ctx->index, mkstr("Vertex input is larger than max allowed binding %u", VSL_MAX_INPUT_INDEX));
		}
		const auto other = shaderInfo_.getInput(index);
		if (other) {
			ERROR(ctx->index, (other->location == index)
				? mkstr("Vertex input slot '%u' is already filled by '%s'", index, other->name.c_str())
				: mkstr("Vertex input slot '%u' overlaps with input '%s'", index, other->name.c_str()));
		}
	}
	else {
		if (index > VSL_MAX_OUTPUT_INDEX) {
			ERROR(ctx->index, mkstr("Fragment output is larger than max allowed binding %u", VSL_MAX_OUTPUT_INDEX));
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
	auto ioVar = parseVariableDeclaration(varDecl, true);
	if (!ioVar.dataType->isNumeric()) {
		ERROR(varDecl->baseType, "Shader interface variables must be a numeric type");
	}

	// Input/Output specific type validation
	if (isIn) {
		if (ioVar.arraySize > VSL_MAX_INPUT_ARRAY_SIZE) {
			ERROR(varDecl->arraySize, mkstr("Vertex input arrays cannot be larger than %u", VSL_MAX_INPUT_ARRAY_SIZE));
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
			ERROR(varDecl->baseType, "Fragment outputs cannot be matrix types");
		}
		if (ioVar.dataType->numeric.dims[0] == 3) {
			ERROR(varDecl->baseType, "Fragment outputs cannot be 3-component vectors");
		}
	}

	// Add to shader info
	InterfaceVariable iovar{ varDecl->name->getText(), index, ioVar.dataType, ioVar.arraySize };
	if (isIn) {
		shaderInfo_.inputs().push_back(iovar);
		ioVar.type = VariableType::Input;
		generator_.emitVertexInput(iovar);
	}
	else {
		shaderInfo_.outputs().push_back(iovar);
		ioVar.type = VariableType::Output;
		generator_.emitFragmentOutput(iovar);
	}
	scopes_.addGlobal(ioVar);

	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(ShaderConstantStatement)
{
	// Check parse status
	if (bool(shaderInfo_.stages())) {
		ERROR(ctx, "All constants must be provided before the first stage function");
	}

	// Validate the variable
	const auto varDecl = ctx->variableDeclaration();
	const auto cVar = parseVariableDeclaration(varDecl, true);
	if (varDecl->arraySize) {
		ERROR(varDecl->arraySize, "Constants cannot be arrays");
	}
	if (!cVar.dataType->isNumeric()) {
		ERROR(varDecl->baseType, "Constants must be a numeric type");
	}
	if ((cVar.dataType->numeric.dims[0] != 1) || (cVar.dataType->numeric.dims[1] != 1)) {
		ERROR(varDecl->baseType, "Constants must be a scalar numeric type");
	}
	if (cVar.dataType->numeric.size != 4) {
		ERROR(varDecl->baseType, "Constants must be a 4-byte scalar type");
	}

	// Parse the literal
	const auto valueLiteral = ParseLiteral(this, ctx->value);
	Constant cnst;
	if (cVar.dataType->baseType == ShaderBaseType::Unsigned) {
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
	else if (cVar.dataType->baseType == ShaderBaseType::Signed) {
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
VISIT_FUNC(ShaderUniformStatement)
{
	// Check parse status
	if (bool(shaderInfo_.stages())) {
		ERROR(ctx, "Uniform must be provided before the first stage function");
	}
	if (shaderInfo_.hasUniform()) {
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
	ShaderType newType{ ShaderBaseType::Uniform, bVar.dataType };
	const auto uType = types_.addType("uniform " + newType.buffer.structType->userStruct.structName, newType);

	// Add to the shader info
	const UniformVariable uvar{ bVar.name, uType };
	shaderInfo_.uniform(uvar);
	generator_.emitUniform(uvar);

	// Add to the scopes
	Variable scopeVar{ VariableType::Binding, bVar.name, uType, 1 };
	scopeVar.extra.binding.slot = 0;
	scopes_.addGlobal(scopeVar);

	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(ShaderBindingStatement)
{
	// Check parse status
	if (bool(shaderInfo_.stages())) {
		ERROR(ctx, "All bindings must be provided before the first stage function");
	}

	// Parse the variable declaration
	const auto varDecl = ctx->variableDeclaration();
	const auto bVar = parseVariableDeclaration(varDecl, true);
	if (bVar.dataType->isNumeric() || bVar.dataType->isBoolean()) {
		ERROR(varDecl->baseType, "Bindings cannot be numeric or boolean types");
	}
	if (bVar.dataType->isStruct()) {
		ERROR(varDecl->baseType, "Bindings that are structs must be a buffer or uniform type");
	}

	// Check for binding limit
	if (bVar.dataType->baseType == ShaderBaseType::Input) {
		if (shaderInfo_.subpassInputs().size() == VSL_MAX_SUBPASS_INPUTS) {
			ERROR(ctx, mkstr("Cannot have more than %u subpass inputs", VSL_MAX_SUBPASS_INPUTS));
		}
	}
	else if (shaderInfo_.bindings().size() == (VSL_MAX_BINDING_INDEX + 1)) {
		ERROR(ctx, mkstr("Cannot have more than %u bindings in a shader", VSL_MAX_BINDING_INDEX + 1));
	}

	// Get the binding slot
	const auto slotLiteral = ParseLiteral(this, ctx->slot);
	if (slotLiteral.isNegative() || (slotLiteral.type == Literal::Float)) {
		ERROR(ctx->slot, "Binding slot index must be non-negative integer");
	}
	if (bVar.dataType->baseType == ShaderBaseType::Input) {
		if (slotLiteral.u >= VSL_MAX_SUBPASS_INPUTS) {
			ERROR(ctx->slot, mkstr("Subpass input index out of range (max %u)", VSL_MAX_SUBPASS_INPUTS - 1));
		}
	}
	else if (slotLiteral.u > VSL_MAX_BINDING_INDEX) {
		ERROR(ctx->slot, mkstr("Slot index out of range (max %u)", VSL_MAX_BINDING_INDEX));
	}
	const auto slotIndex = uint8(slotLiteral.u);

	// Check and add to the shader info
	if (bVar.dataType->baseType == ShaderBaseType::Input) {
		if (shaderInfo_.subpassInputs().size() != slotIndex) {
			ERROR(ctx->slot, "Subpass input indices must be contiguous");
		}
		const SubpassInput si{ 
			bVar.name, bVar.dataType->image.texel.type, bVar.dataType->image.texel.components, slotIndex
		};
		generator_.emitSubpassInput(si);
		shaderInfo_.subpassInputs().push_back(si);
	}
	else {
		if (shaderInfo_.getBinding(slotIndex)) {
			ERROR(ctx->slot, mkstr("Binding slot %s is already filled by another binding", ctx->slot->getText().c_str()));
		}
		const BindingVariable bvar{ bVar.name, bVar.dataType, slotIndex };
		generator_.emitBinding(bvar);
		shaderInfo_.bindings().push_back(bvar);
	}

	// Add to the scope manager
	Variable scopeVar = bVar;
	scopeVar.type = VariableType::Binding;
	scopeVar.extra.binding.slot = slotIndex;
	scopes_.addGlobal(scopeVar);

	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(ShaderLocalStatement)
{
	// Check parse status
	if (bool(shaderInfo_.stages())) {
		ERROR(ctx, "All locals must be provided before the first stage function");
	}

	// Parse and validate variable
	const auto isFlat = !!ctx->KW_FLAT();
	const auto varDecl = ctx->variableDeclaration();
	const auto lVar = parseVariableDeclaration(varDecl, true);
	if (lVar.arraySize != 1) {
		ERROR(varDecl->arraySize, "Shader locals cannot be arrays");
	}
	if (!lVar.dataType->isNumeric() || (lVar.dataType->numeric.dims[1] != 1)) {
		ERROR(varDecl->baseType, "Shader locals must be numeric scalars or vectors");
	}
	if ((lVar.dataType->baseType == ShaderBaseType::Unsigned) || (lVar.dataType->baseType == ShaderBaseType::Signed)) {
		if (!isFlat) {
			ERROR(varDecl->baseType, "Shader locals with integer types must be declared as 'flat'");
		}
	}

	// Parse and validate stages
	const auto pStage = StrToShaderStage(ctx->pstage->getText());
	if (pStage == ShaderStages::None) {
		ERROR(ctx->pstage, mkstr("Unknown shader stage '%s'", ctx->pstage->getText().c_str()));
	}
	if ((pStage == ShaderStages::TessControl) || (pStage == ShaderStages::TessEval) || 
			(pStage == ShaderStages::Geometry)) {
		ERROR(ctx->pstage, "Unsupported producer shader stage in local");
	}

	// Add variable
	Variable newVar = lVar;
	newVar.type = VariableType::Local;
	newVar.extra.local.pStage = pStage;
	newVar.extra.local.flat = isFlat;
	scopes_.addGlobal(newVar);

	// Emit
	generator_.emitLocal(newVar);

	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(ShaderStageFunction)
{
	// Validate stage
	const auto stage = StrToShaderStage(ctx->stage->getText());
	if (stage == ShaderStages::None) {
		ERROR(ctx->stage, mkstr("Unknown shader stage '%s' for function", ctx->stage->getText().c_str()));
	}
	if ((shaderInfo_.stages() & stage) == stage) {
		ERROR(ctx->stage, mkstr("Duplicate shader function for stage '%s'", ctx->stage->getText().c_str()));
	}
	
	// Temp remove support for tess/geom until the system is well built for vert/frag
	if (stage == ShaderStages::TessControl) {
		ERROR(ctx->stage, "Tessellation control stage is not yet supported");
	}
	if (stage == ShaderStages::TessEval) {
		ERROR(ctx->stage, "Tessellation evaluation stage is not yet supported");
	}
	if (stage == ShaderStages::Geometry) {
		ERROR(ctx->stage, "Geometry stage is not yet supported");
	}

	// If this is the first shader stage, do some global variable finialization
	if (!bool(shaderInfo_.stages())) {
		generator_.emitBindingIndices(shaderInfo_.getMaxBindingIndex());
	}

	// Push the global scope for the stage
	scopes_.pushGlobalScope(stage);
	generator_.setCurrentStage(stage);

	// Visit the function statements
	currentStage_ = stage;
	for (const auto stmt : ctx->statementBlock()->statement()) {
		visit(stmt);
	}
	currentStage_ = ShaderStages::None;

	// Pop the global scope
	generator_.setCurrentStage(ShaderStages::None);
	scopes_.popScope();

	// Update shader info
	shaderInfo_.addStage(stage);

	return nullptr;
}

} // namespace vsl
