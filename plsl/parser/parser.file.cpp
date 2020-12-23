/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./parser.hpp"

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
	for (const auto field : ctx->fieldDeclaration()) {
		// Get field info
		const auto tName = field->type->getText();
		const auto fName = field->name->getText();

		// Check name
		if (structType.hasMember(fName)) {
			ERROR(field->name, mkstr("Duplicate field name '%s'", fName.c_str()));
		}

		// Check field type
		const auto fType = types_.getType(tName);
		if (!fType) {
			ERROR(field->type, mkstr("Unknown type '%s' for field '%s'", tName.c_str(), fName.c_str()));
		}
		if (!fType->isNumeric()) {
			ERROR(field->type, mkstr("Invalid field '%s' - type fields must be numeric", fName.c_str()));
		}

		// Check for array size
		const auto arrSize = field->arraySize
			? ParseLiteral(this, field->arraySize)
			: Literal{ 1ull };
		if (arrSize.isNegative() || arrSize.isZero()) {
			ERROR(field->arraySize, "Array size cannot be negative or zero");
		}
		if (arrSize.u > PLSL_MAX_ARRAY_SIZE) {
			ERROR(field->arraySize, mkstr("Array size cannot be larger than %u", PLSL_MAX_ARRAY_SIZE));
		}

		// Add the member
		StructMember mem{};
		mem.name = fName;
		mem.baseType = fType->baseType;
		mem.size = fType->numeric.size;
		mem.dims[0] = fType->numeric.dims[0];
		mem.dims[1] = fType->numeric.dims[1];
		mem.arraySize = uint8_t(arrSize.u);
		structType.userStruct.members.push_back(mem);
	}

	// Add the struct type to the type manager
	types_.addType(structType.userStruct.structName, structType);

	return nullptr;
}

} // namespace plsl
