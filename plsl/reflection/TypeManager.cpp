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
	{ "int",   { ShaderBaseType::SInteger, 4, 1, 1 } }, { "int2",  { ShaderBaseType::SInteger, 4, 2, 1 } },
	{ "int3",  { ShaderBaseType::SInteger, 4, 3, 1 } }, { "int4",  { ShaderBaseType::SInteger, 4, 4, 1 } },
	{ "uint",  { ShaderBaseType::UInteger, 4, 1, 1 } }, { "uint2", { ShaderBaseType::UInteger, 4, 2, 1 } },
	{ "uint3", { ShaderBaseType::UInteger, 4, 3, 1 } }, { "uint4", { ShaderBaseType::UInteger, 4, 4, 1 } },
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
	// Sampler
	{ "Sampler",       { ShaderBaseType::Sampler, ImageDims::None } },
	{ "SamplerShadow", { ShaderBaseType::Sampler, ImageDims::Shadow } },
	// Bound Sampler
	{ "Sampler1D",        { ShaderBaseType::BoundSampler, ImageDims::E1D } },
	{ "Sampler2D",        { ShaderBaseType::BoundSampler, ImageDims::E2D } },
	{ "Sampler3D",        { ShaderBaseType::BoundSampler, ImageDims::E3D } },
	{ "Sampler1DArray",   { ShaderBaseType::BoundSampler, ImageDims::E1DArray } },
	{ "Sampler2DArray",   { ShaderBaseType::BoundSampler, ImageDims::E2DArray } },
	{ "SamplerCube",      { ShaderBaseType::BoundSampler, ImageDims::Cube } },
	{ "SamplerCubeArray", { ShaderBaseType::BoundSampler, ImageDims::CubeArray } },
	// Texture
	{ "Texture1D",         { ShaderBaseType::Texture, ImageDims::E1D, ShaderBaseType::Float, 4 } },
	{ "Texture2D",         { ShaderBaseType::Texture, ImageDims::E2D, ShaderBaseType::Float, 4 } },
	{ "Texture3D",         { ShaderBaseType::Texture, ImageDims::E3D, ShaderBaseType::Float, 4 } },
	{ "Texture1DArray",    { ShaderBaseType::Texture, ImageDims::E1DArray, ShaderBaseType::Float, 4 } },
	{ "Texture2DArray",    { ShaderBaseType::Texture, ImageDims::E2DArray, ShaderBaseType::Float, 4 } },
	{ "TextureCube",       { ShaderBaseType::Texture, ImageDims::Cube, ShaderBaseType::Float, 4 } },
	{ "TextureCubeArray",  { ShaderBaseType::Texture, ImageDims::CubeArray, ShaderBaseType::Float, 4 } },
	{ "ITexture1D",        { ShaderBaseType::Texture, ImageDims::E1D, ShaderBaseType::SInteger, 4 } },
	{ "ITexture2D",        { ShaderBaseType::Texture, ImageDims::E2D, ShaderBaseType::SInteger, 4 } },
	{ "ITexture3D",        { ShaderBaseType::Texture, ImageDims::E3D, ShaderBaseType::SInteger, 4 } },
	{ "ITexture1DArray",   { ShaderBaseType::Texture, ImageDims::E1DArray, ShaderBaseType::SInteger, 4 } },
	{ "ITexture2DArray",   { ShaderBaseType::Texture, ImageDims::E2DArray, ShaderBaseType::SInteger, 4 } },
	{ "ITextureCube",      { ShaderBaseType::Texture, ImageDims::Cube, ShaderBaseType::SInteger, 4 } },
	{ "ITextureCubeArray", { ShaderBaseType::Texture, ImageDims::CubeArray, ShaderBaseType::SInteger, 4 } },
	{ "UTexture1D",        { ShaderBaseType::Texture, ImageDims::E1D, ShaderBaseType::UInteger, 4 } },
	{ "UTexture2D",        { ShaderBaseType::Texture, ImageDims::E2D, ShaderBaseType::UInteger, 4 } },
	{ "UTexture3D",        { ShaderBaseType::Texture, ImageDims::E3D, ShaderBaseType::UInteger, 4 } },
	{ "UTexture1DArray",   { ShaderBaseType::Texture, ImageDims::E1DArray, ShaderBaseType::UInteger, 4 } },
	{ "UTexture2DArray",   { ShaderBaseType::Texture, ImageDims::E2DArray, ShaderBaseType::UInteger, 4 } },
	{ "UTextureCube",      { ShaderBaseType::Texture, ImageDims::Cube, ShaderBaseType::UInteger, 4 } },
	{ "UTextureCubeArray", { ShaderBaseType::Texture, ImageDims::CubeArray, ShaderBaseType::UInteger, 4 } },
	// Image (Incomplete Descriptions)
	{ "Image1D",        { ShaderBaseType::Image, ImageDims::E1D } },
	{ "Image2D",        { ShaderBaseType::Image, ImageDims::E2D } },
	{ "Image3D",        { ShaderBaseType::Image, ImageDims::E3D } },
	{ "Image1DArray",   { ShaderBaseType::Image, ImageDims::E1DArray } },
	{ "Image2DArray",   { ShaderBaseType::Image, ImageDims::E2DArray } },
	{ "ImageCube",      { ShaderBaseType::Image, ImageDims::Cube } },
	{ "ImageCubeArray", { ShaderBaseType::Image, ImageDims::CubeArray } },
	// Buffer Types (Incomplete Descriptions)
	{ "Uniform",  { ShaderBaseType::Uniform, "" } },
	{ "ROBuffer", { ShaderBaseType::ROBuffer, "" } },
	{ "RWBuffer", { ShaderBaseType::RWBuffer, "" } },
	{ "ROTexels", { ShaderBaseType::ROTexels, ImageDims::Buffer } },
	{ "RWTexels", { ShaderBaseType::RWTexels, ImageDims::Buffer } },
	// Subpass Input
	{ "Input",  { ShaderBaseType::Input, ImageDims::E2D, ShaderBaseType::Float, 4 } },
	{ "IInput", { ShaderBaseType::Input, ImageDims::E2D, ShaderBaseType::SInteger, 4 } },
	{ "UInput", { ShaderBaseType::Input, ImageDims::E2D, ShaderBaseType::UInteger, 4 } },
};

} // namespace plsl
