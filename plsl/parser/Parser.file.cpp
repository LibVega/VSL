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

	// Validate the name
	const auto varDecl = ctx->variableDeclaration();
	if (scopes_.hasGlobal(varDecl->name->getText())) {
		ERROR(varDecl->name, mkstr("A variable with the name '%s' already exists", varDecl->name->getText().c_str()));
	}

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
	const auto ioType = types_.getType(varDecl->type->getText());
	const auto arrSizeLiteral = varDecl->arraySize ? ParseLiteral(this, varDecl->arraySize) : Literal{ 1ull };
	if (!ioType) {
		ERROR(varDecl->type, mkstr("Unknown type '%s'", varDecl->type->getText().c_str()));
	}
	if (!ioType->isNumeric()) {
		ERROR(varDecl->type, "Shader interface variables must be a numeric type");
	}
	if (arrSizeLiteral.isNegative() || arrSizeLiteral.isZero()) {
		ERROR(varDecl->arraySize, "Array size cannot be zero or negative");
	}
	const auto arrSize = uint32(arrSizeLiteral.u);

	// Input/Output specific type validation
	if (isIn) {
		if (arrSize > PLSL_MAX_INPUT_ARRAY_SIZE) {
			ERROR(varDecl->arraySize, mkstr("Vertex input arrays cannot be larger than %u", PLSL_MAX_INPUT_ARRAY_SIZE));
		}
		if ((arrSize != 1) && (ioType->numeric.dims[1] != 1)) {
			ERROR(varDecl->arraySize, "Vertex inputs that are matrix types cannot be arrays");
		}
	}
	else { // Outputs must be non-arrays, and either scalars or vectors
		if (arrSize != 1) {
			ERROR(varDecl->arraySize, "Fragment outputs cannot be arrays");
		}
		if (ioType->numeric.dims[1] != 1) {
			ERROR(varDecl->type, "Fragment outputs cannot be matrix types");
		}
	}

	// Add to shader info
	InterfaceVariable iovar{ varDecl->name->getText(), index, *ioType, uint8(arrSize) };
	if (isIn) {
		shaderInfo_.inputs().push_back(iovar);
	}
	else {
		shaderInfo_.outputs().push_back(iovar);
	}
	scopes_.addGlobal({ isIn ? VariableType::Input : VariableType::Output, iovar.name, ioType, iovar.arraySize });

	return nullptr;
}

} // namespace plsl
