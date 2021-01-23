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
	StructType structType{ typeName, members };
	if (structType.size() > Shader::MAX_STRUCT_SIZE) {
		ERROR(ctx->name, mkstr("Struct types cannot be larger than %u bytes", Shader::MAX_STRUCT_SIZE));
	}

	// Add the struct type
	shader_->types().addStructType(typeName, structType);

	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(ShaderInputOutputStatement)
{
	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(ShaderUniformStatement)
{
	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(ShaderBindingStatement)
{
	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(ShaderLocalStatement)
{
	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(ShaderSubpassInputStatement)
{
	return nullptr;
}

// ====================================================================================================================
VISIT_FUNC(ShaderStageFunction)
{
	return nullptr;
}

} // namespace vsl
