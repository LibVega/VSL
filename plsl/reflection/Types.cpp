/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./Types.hpp"
#include "../glsl/NameHelper.hpp"

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

// ====================================================================================================================
string ShaderType::getPLSLName() const
{
	switch (baseType)
	{
	case ShaderBaseType::Void: return "void";
	case ShaderBaseType::Boolean: {
		return (numeric.dims[0] == 1) ? "bool" : mkstr("bool%u", uint32(numeric.dims[0]));
	} break;
	case ShaderBaseType::Signed: {
		return (numeric.dims[0] == 1) ? "int" : mkstr("int%u", uint32(numeric.dims[0]));
	} break;
	case ShaderBaseType::Unsigned: {
		return (numeric.dims[0] == 1) ? "uint" : mkstr("uint%u", uint32(numeric.dims[0]));
	} break;
	case ShaderBaseType::Float: {
		if (numeric.dims[1] == 1) {
			return (numeric.dims[0] == 1) ? "float" : mkstr("float%u", uint32(numeric.dims[0]));
		}
		else {
			return mkstr("float%ux%u", uint32(numeric.dims[1]), uint32(numeric.dims[0]));
		}
	} break;
	case ShaderBaseType::Sampler: {
		const string prefix =
			(image.texel.type == ShaderBaseType::Signed) ? "I" :
			(image.texel.type == ShaderBaseType::Unsigned) ? "U" : "";
		const auto suffix = NameHelper::GetImageDimsPostfix(image.dims);
		return prefix + "Sampler" + suffix;
	} break;
	case ShaderBaseType::Image: {
		const auto suffix = NameHelper::GetImageDimsPostfix(image.dims);
		const auto format = NameHelper::GetImageTexelFormat(image.texel.type, image.texel.size, image.texel.components);
		return "Image" + suffix + "<" + format + ">";
	} break;
	case ShaderBaseType::Uniform: return "Uniform<" + buffer.structName + ">";
	case ShaderBaseType::ROBuffer: return "ROBuffer<" + buffer.structName + ">";
	case ShaderBaseType::RWBuffer: return "RWBuffer<" + buffer.structName + ">";
	case ShaderBaseType::ROTexels: return "ROTexels";
	case ShaderBaseType::RWTexels: {
		const auto format = NameHelper::GetImageTexelFormat(image.texel.type, image.texel.size, image.texel.components);
		return "RWTexels<" + format + ">";
	} break;
	case ShaderBaseType::Input: {
		const string prefix =
			(image.texel.type == ShaderBaseType::Signed) ? "I" :
			(image.texel.type == ShaderBaseType::Unsigned) ? "U" : "";
		return prefix + "Input";
	} break;
	case ShaderBaseType::Struct: return userStruct.structName;
	default: return "INVALID";
	}
}

// ====================================================================================================================
string ShaderType::getGLSLName() const
{
	switch (baseType)
	{
	case ShaderBaseType::Void: return "void";
	case ShaderBaseType::Boolean:
	case ShaderBaseType::Signed:
	case ShaderBaseType::Unsigned:
	case ShaderBaseType::Float: 
		return NameHelper::GetNumericTypeName(baseType, numeric.size, numeric.dims[0], numeric.dims[1]);
	case ShaderBaseType::Sampler:
	case ShaderBaseType::Image:
	case ShaderBaseType::ROTexels:
	case ShaderBaseType::RWTexels:
	case ShaderBaseType::Input: {
		string extra{};
		return NameHelper::GetBindingTypeName(this, &extra);
	} break;
	case ShaderBaseType::Struct: return userStruct.structName + "_t";
	case ShaderBaseType::Uniform:
	case ShaderBaseType::ROBuffer:
	case ShaderBaseType::RWBuffer: return buffer.structName + "_t";
	default: return "INVALID";
	}
}

} // namespace plsl
