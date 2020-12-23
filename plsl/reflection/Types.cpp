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
bool ShaderType::isComplete() const
{
	switch (baseType)
	{
	case ShaderBaseType::Void: 
		return true;
	case ShaderBaseType::Boolean:
	case ShaderBaseType::UInteger:
	case ShaderBaseType::SInteger:
	case ShaderBaseType::Float:
		return (numeric.size != 0) && (numeric.dims[0] != 0) && (numeric.dims[1] != 0);
	case ShaderBaseType::Sampler: 
		return true;
	case ShaderBaseType::BoundSampler: 
		return (image.dims != ImageDims::None);
	case ShaderBaseType::Texture:
	case ShaderBaseType::Image:
	case ShaderBaseType::Input:
		return (image.dims != ImageDims::None) && (image.texel.type != ShaderBaseType::Void) &&
			(image.texel.size != 0) && (image.texel.components != 0);
	case ShaderBaseType::Uniform:
	case ShaderBaseType::ROBuffer:
	case ShaderBaseType::RWBuffer:
	case ShaderBaseType::ROTexels:
	case ShaderBaseType::RWTexels:
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
uint32 ShaderType::getBindingCount() const
{
	if (!isNumeric()) {
		return 0;
	}

	const auto totalSize = numeric.size * numeric.dims[0];
	static const uint32 SLOT_SIZE = 16; // 16 bytes per binding slot for inputs
	return ((totalSize > SLOT_SIZE) ? 2 : 1) * numeric.dims[1];
}

} // namespace plsl
