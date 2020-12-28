/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./TypeManager.hpp"


namespace plsl
{

// ====================================================================================================================
TypeManager::TypeManager()
{

}

// ====================================================================================================================
TypeManager::~TypeManager()
{

}

// ====================================================================================================================
const ShaderType* TypeManager::addType(const string& name, const ShaderType& type)
{
	auto it = addedTypes_.find(name);
	if (it == addedTypes_.end()) {
		return &(addedTypes_[name] = type);
	}
	lastError_ = mkstr("type name '%s' already exists", name.c_str());
	return nullptr;
}

// ====================================================================================================================
const ShaderType* TypeManager::getType(const string& typeName) const
{
	auto it = addedTypes_.find(typeName);
	if (it != addedTypes_.end()) {
		return &(it->second);
	}
	it = BuiltinTypes_.find(typeName);
	if (it != BuiltinTypes_.end()) {
		return &(it->second);
	}
	lastError_ = mkstr("no type with name '%s' was found", typeName.c_str());
	return nullptr;
}

// ====================================================================================================================
const ShaderType* TypeManager::getOrAddType(const string& typeName)
{
	// Check if it exists already
	if (const auto type = getType(typeName); type && type->isComplete()) {
		return type;
	}

	// Get the subtype (if no subtype, return error because all non-subtype types are already known)
	const auto stIndex = typeName.find('<');
	if (stIndex == string::npos) {
		lastError_ = mkstr("unknown non-subtyped type name '%s'", typeName.c_str());
		return nullptr;
	}
	const auto baseType = getType(typeName.substr(0, stIndex));
	if (!baseType) {
		lastError_ = mkstr("unknown subtype base type '%s'", typeName.substr(0, stIndex).c_str());
		return nullptr;
	}
	if (!baseType->hasSubtype()) {
		lastError_ = mkstr("cannot apply subtype to invalid base type '%s'", typeName.substr(0, stIndex).c_str());
		return nullptr;
	}
	const auto subTypeName = typeName.substr(stIndex + 1, typeName.length() - stIndex - 2);
	const auto subType = getType(subTypeName);
	
	// Subtype validation
	if (!subType) {
		lastError_ = mkstr("could not find subtype '%s'", subTypeName.c_str());
		return nullptr;
	}
	if ((baseType->baseType == ShaderBaseType::Image) || (baseType->baseType == ShaderBaseType::RWTexels)) {
		if (!subType->isNumeric() || (subType->numeric.dims[1] != 1)) {
			lastError_ = "texel-like subtype must be a numeric scalar or vector";
			return nullptr;
		}
		if (subType->numeric.dims[0] == 3) {
			lastError_ = "texel-like subtype cannot be a three-component vector";
			return nullptr;
		}
	}
	else { // Buffer types
		if (!subType->isStruct()) {
			lastError_ = "buffer subtype must be a user-defined struct";
			return nullptr;
		}
	}

	// Create the new type
	ShaderType newType = *baseType;
	if ((baseType->baseType == ShaderBaseType::Image) || (baseType->baseType == ShaderBaseType::RWTexels)) {
		newType.image.texel.type = subType->baseType;
		newType.image.texel.size = subType->numeric.size;
		newType.image.texel.components = subType->numeric.dims[0];
	}
	else {
		newType.buffer.structName = subTypeName;
	}
	return &(addedTypes_[typeName] = newType);
}

// ====================================================================================================================
const ShaderType* TypeManager::getNumericType(ShaderBaseType type, uint32 dim0, uint32 dim1) const
{
	for (const auto& pair : BuiltinTypes_) {
		if ((pair.second.baseType == type) && (pair.second.numeric.dims[0] == dim0) &&
				(pair.second.numeric.dims[1] == dim1)) {
			return &(pair.second);
		}
	}
	return nullptr;
}

// ====================================================================================================================
const std::unordered_map<string, ShaderType> TypeManager::BuiltinTypes_ {
	{ "void", { } },
	// Boolean
	{ "bool",  { ShaderBaseType::Boolean, 4, 1, 1 } }, { "bool2", { ShaderBaseType::Boolean, 4, 2, 1 } },
	{ "bool3", { ShaderBaseType::Boolean, 4, 3, 1 } }, { "bool4", { ShaderBaseType::Boolean, 4, 4, 1 } },
	// Integer
	{ "int",   { ShaderBaseType::Signed, 4, 1, 1 } }, { "int2",  { ShaderBaseType::Signed, 4, 2, 1 } },
	{ "int3",  { ShaderBaseType::Signed, 4, 3, 1 } }, { "int4",  { ShaderBaseType::Signed, 4, 4, 1 } },
	{ "uint",  { ShaderBaseType::Unsigned, 4, 1, 1 } }, { "uint2", { ShaderBaseType::Unsigned, 4, 2, 1 } },
	{ "uint3", { ShaderBaseType::Unsigned, 4, 3, 1 } }, { "uint4", { ShaderBaseType::Unsigned, 4, 4, 1 } },
	// Floating
	{ "float",  { ShaderBaseType::Float, 4, 1, 1 } }, { "float2", { ShaderBaseType::Float, 4, 2, 1 } },
	{ "float3", { ShaderBaseType::Float, 4, 3, 1 } }, { "float4", { ShaderBaseType::Float, 4, 4, 1 } },
	// Matrices
	{ "float2x2", { ShaderBaseType::Float, 4, 2, 2 } },
	{ "float3x3", { ShaderBaseType::Float, 4, 3, 3 } },
	{ "float4x4", { ShaderBaseType::Float, 4, 4, 4 } },
	{ "float2x3", { ShaderBaseType::Float, 4, 3, 2 } }, { "float3x2", { ShaderBaseType::Float, 4, 2, 3 } },
	{ "float2x4", { ShaderBaseType::Float, 4, 4, 2 } }, { "float4x2", { ShaderBaseType::Float, 4, 2, 4 } },
	{ "float3x4", { ShaderBaseType::Float, 4, 4, 3 } }, { "float4x3", { ShaderBaseType::Float, 4, 3, 4 } },
	// Bound Sampler
	{ "Sampler1D",        { ShaderBaseType::Sampler, ImageDims::E1D } },
	{ "Sampler2D",        { ShaderBaseType::Sampler, ImageDims::E2D } },
	{ "Sampler3D",        { ShaderBaseType::Sampler, ImageDims::E3D } },
	{ "Sampler1DArray",   { ShaderBaseType::Sampler, ImageDims::E1DArray } },
	{ "Sampler2DArray",   { ShaderBaseType::Sampler, ImageDims::E2DArray } },
	{ "SamplerCube",      { ShaderBaseType::Sampler, ImageDims::Cube } },
	// ROTexture
	/*{ "Texture1D",         { ShaderBaseType::ROTexture, ImageDims::E1D, ShaderBaseType::Float, 4 } },
	{ "Texture2D",         { ShaderBaseType::ROTexture, ImageDims::E2D, ShaderBaseType::Float, 4 } },
	{ "Texture3D",         { ShaderBaseType::ROTexture, ImageDims::E3D, ShaderBaseType::Float, 4 } },
	{ "Texture1DArray",    { ShaderBaseType::ROTexture, ImageDims::E1DArray, ShaderBaseType::Float, 4 } },
	{ "Texture2DArray",    { ShaderBaseType::ROTexture, ImageDims::E2DArray, ShaderBaseType::Float, 4 } },
	{ "TextureCube",       { ShaderBaseType::ROTexture, ImageDims::Cube, ShaderBaseType::Float, 4 } },
	{ "ITexture1D",        { ShaderBaseType::ROTexture, ImageDims::E1D, ShaderBaseType::Signed, 4 } },
	{ "ITexture2D",        { ShaderBaseType::ROTexture, ImageDims::E2D, ShaderBaseType::Signed, 4 } },
	{ "ITexture3D",        { ShaderBaseType::ROTexture, ImageDims::E3D, ShaderBaseType::Signed, 4 } },
	{ "ITexture1DArray",   { ShaderBaseType::ROTexture, ImageDims::E1DArray, ShaderBaseType::Signed, 4 } },
	{ "ITexture2DArray",   { ShaderBaseType::ROTexture, ImageDims::E2DArray, ShaderBaseType::Signed, 4 } },
	{ "ITextureCube",      { ShaderBaseType::ROTexture, ImageDims::Cube, ShaderBaseType::Signed, 4 } },
	{ "UTexture1D",        { ShaderBaseType::ROTexture, ImageDims::E1D, ShaderBaseType::Unsigned, 4 } },
	{ "UTexture2D",        { ShaderBaseType::ROTexture, ImageDims::E2D, ShaderBaseType::Unsigned, 4 } },
	{ "UTexture3D",        { ShaderBaseType::ROTexture, ImageDims::E3D, ShaderBaseType::Unsigned, 4 } },
	{ "UTexture1DArray",   { ShaderBaseType::ROTexture, ImageDims::E1DArray, ShaderBaseType::Unsigned, 4 } },
	{ "UTexture2DArray",   { ShaderBaseType::ROTexture, ImageDims::E2DArray, ShaderBaseType::Unsigned, 4 } },
	{ "UTextureCube",      { ShaderBaseType::ROTexture, ImageDims::Cube, ShaderBaseType::Unsigned, 4 } },*/
	// Image (Incomplete Descriptions)
	{ "Image1D",        { ShaderBaseType::Image, ImageDims::E1D } },
	{ "Image2D",        { ShaderBaseType::Image, ImageDims::E2D } },
	{ "Image3D",        { ShaderBaseType::Image, ImageDims::E3D } },
	{ "Image1DArray",   { ShaderBaseType::Image, ImageDims::E1DArray } },
	{ "Image2DArray",   { ShaderBaseType::Image, ImageDims::E2DArray } },
	{ "ImageCube",      { ShaderBaseType::Image, ImageDims::Cube } },
	// Buffer Types (Incomplete Descriptions, except for RO*Texels)
	{ "Uniform",   { ShaderBaseType::Uniform, "" } },
	{ "ROBuffer",  { ShaderBaseType::ROBuffer, "" } },
	{ "RWBuffer",  { ShaderBaseType::RWBuffer, "" } },
	{ "ROTexels",  { ShaderBaseType::ROTexels, ImageDims::Buffer, ShaderBaseType::Float, 4 } },
	{ "ROITexels", { ShaderBaseType::ROTexels, ImageDims::Buffer, ShaderBaseType::Signed, 4 } },
	{ "ROUTexels", { ShaderBaseType::ROTexels, ImageDims::Buffer, ShaderBaseType::Unsigned, 4 } },
	{ "RWTexels",  { ShaderBaseType::RWTexels, ImageDims::Buffer } },
	// Subpass Input
	{ "Input",  { ShaderBaseType::Input, ImageDims::E2D, ShaderBaseType::Float, 4 } },
	{ "IInput", { ShaderBaseType::Input, ImageDims::E2D, ShaderBaseType::Signed, 4 } },
	{ "UInput", { ShaderBaseType::Input, ImageDims::E2D, ShaderBaseType::Unsigned, 4 } },
};

} // namespace plsl
