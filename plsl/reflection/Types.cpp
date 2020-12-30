/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./Types.hpp"

#include <algorithm>


namespace plsl
{

// ====================================================================================================================
// ====================================================================================================================
uint32 GetImageDimsComponentCount(ImageDims dims)
{
	switch (dims)
	{
	case ImageDims::E1D: return 1;
	case ImageDims::E2D: return 2;
	case ImageDims::E3D: return 3;
	case ImageDims::E1DArray: return 2;
	case ImageDims::E2DArray: return 3;
	case ImageDims::Cube: return 2;
	case ImageDims::Buffer: return 1;
	default: return 0;
	}
}

// ====================================================================================================================
// ====================================================================================================================
bool ShaderType::hasSubtype() const
{
	return 
		(baseType == ShaderBaseType::Image) || (baseType == ShaderBaseType::RWTexels) || 
		(baseType == ShaderBaseType::Uniform) || (baseType == ShaderBaseType::ROBuffer) || 
		(baseType == ShaderBaseType::RWBuffer);
}

// ====================================================================================================================
bool ShaderType::isComplete() const
{
	switch (baseType)
	{
	case ShaderBaseType::Void: 
	case ShaderBaseType::Struct:
		return true;
	case ShaderBaseType::Boolean:
	case ShaderBaseType::Unsigned:
	case ShaderBaseType::Signed:
	case ShaderBaseType::Float:
		return (numeric.size != 0) && (numeric.dims[0] != 0) && (numeric.dims[1] != 0);
	case ShaderBaseType::ROTexels:
		return true;
	case ShaderBaseType::Sampler: 
		return (image.dims != ImageDims::None);
	case ShaderBaseType::Image:
	case ShaderBaseType::Input:
	case ShaderBaseType::RWTexels:
		return (image.dims != ImageDims::None) && (image.texel.type != ShaderBaseType::Void) &&
			(image.texel.size != 0) && (image.texel.components != 0);
	case ShaderBaseType::Uniform:
	case ShaderBaseType::ROBuffer:
	case ShaderBaseType::RWBuffer:
		return !buffer.structName.empty();
	}
	return false;
}

// ====================================================================================================================
bool ShaderType::hasMember(const string& memberName) const
{
	if (baseType != ShaderBaseType::Struct) {
		return false;
	}

	const auto it =
		std::find_if(userStruct.members.begin(), userStruct.members.end(), [&memberName](const StructMember& mem) {
			return mem.name == memberName;
		});
	return it != userStruct.members.end();
}

// ====================================================================================================================
const StructMember* ShaderType::getMember(const string& memberName) const
{
	if (baseType != ShaderBaseType::Struct) {
		return nullptr;
	}

	const auto it =
		std::find_if(userStruct.members.begin(), userStruct.members.end(), [&memberName](const StructMember& mem) {
			return mem.name == memberName;
		});
	return (it != userStruct.members.end()) ? &(*it) : nullptr;
}

// ====================================================================================================================
uint32 ShaderType::getBindingCount() const
{
	if (!isNumeric()) {
		return 0;
	}

	const auto totalSize = numeric.size * numeric.dims[0];
	static const uint32 SLOT_SIZE = 16; // 16 bytes per binding slot for inputs
	return ((totalSize > SLOT_SIZE) ? 2 : 1) * numeric.dims[1];
}

// ====================================================================================================================
bool ShaderType::hasImplicitCast(const ShaderType* target) const
{
	// Only numerics can cast
	if (!isNumeric() || !target->isNumeric()) {
		return false;
	}

	// Numeric dimensions must match for casting
	if ((numeric.dims[0] != target->numeric.dims[0]) || (numeric.dims[1] != target->numeric.dims[1])) {
		return false;
	}

	// All numeric types can cast to float
	if (target->baseType == ShaderBaseType::Float) {
		return true;
	}

	// Self-cast works (not really a cast)
	if (target->baseType == baseType) {
		return true;
	}

	// This can only match signed -> unsigned, which is implicit
	return (target->baseType == ShaderBaseType::Unsigned);
}

} // namespace plsl
