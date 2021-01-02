/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./NameHelper.hpp"

#include <algorithm>


namespace vsl
{

// ====================================================================================================================
string NameHelper::GetGeneralTypeName(const ShaderType* type)
{
	string extra{};
	return
		(type->isNumeric() || (type->baseType == ShaderBaseType::Boolean)) ?
			GetNumericTypeName(type->baseType, type->numeric.size, type->numeric.dims[0], type->numeric.dims[1]) :
		type->isStruct() ? (type->userStruct.structName) :
		GetBindingTypeName(type, &extra);
}

// ====================================================================================================================
string NameHelper::GetNumericTypeName(ShaderBaseType type, uint8 size, uint8 dim0, uint8 dim1)
{
	// TODO: Add more once non-4 sizes are supported
	switch (type)
	{
	case ShaderBaseType::Boolean:
		return (dim0 == 1) ? "bool" : (dim0 == 2) ? "bvec2" : (dim0 == 3) ? "bvec3" : (dim0 == 4) ? "bvec4" : "";
	case ShaderBaseType::Unsigned:
		return (dim0 == 1) ? "uint" : (dim0 == 2) ? "uvec2" : (dim0 == 3) ? "uvec3" : (dim0 == 4) ? "uvec4" : "";
	case ShaderBaseType::Signed:
		return (dim0 == 1) ? "int" : (dim0 == 2) ? "ivec2" : (dim0 == 3) ? "ivec3" : (dim0 == 4) ? "ivec4" : "";
	case ShaderBaseType::Float: {
		if (dim1 == 1) {
			return (dim0 == 1) ? "float" : (dim0 == 2) ? "vec2" : (dim0 == 3) ? "vec3" : (dim0 == 4) ? "vec4" : "";
		}
		if (dim1 == 2) {
			return (dim0 == 2) ? "mat2x2" : (dim0 == 3) ? "mat2x3" : (dim0 == 4) ? "mat2x4" : "";
		}
		if (dim1 == 3) {
			return (dim0 == 2) ? "mat3x2" : (dim0 == 3) ? "mat3x3" : (dim0 == 4) ? "mat3x4" : "";
		}
		if (dim1 == 4) {
			return (dim0 == 2) ? "mat4x2" : (dim0 == 3) ? "mat4x3" : (dim0 == 4) ? "mat4x4" : "";
		}
	} break;
	}

	return "";
}

// ====================================================================================================================
string NameHelper::GetBindingTypeName(const ShaderType* type, string* extra)
{
	switch (type->baseType)
	{
	case ShaderBaseType::Sampler: {
		const auto dimText = GetImageDimsPostfix(type->image.dims);
		if (dimText.empty()) {
			return "";
		}
		const string prefix = GetImageTexelPrefix(type->image.texel.type);
		*extra = "";
		return prefix + "sampler" + dimText;
	} break;
	case ShaderBaseType::Image: {
		const auto dimText = GetImageDimsPostfix(type->image.dims);
		if (dimText.empty()) {
			return "";
		}
		const string prefix = GetImageTexelPrefix(type->image.texel.type);
		*extra = GetImageTexelFormat(type->image.texel.type, type->image.texel.size, type->image.texel.components);
		if (extra->empty()) {
			*extra = "!";
		}
		return prefix + "image" + dimText;
	} break;
	case ShaderBaseType::Uniform: {
		*extra = "std140";
		return " ";
	} break;
	case ShaderBaseType::ROBuffer: {
		*extra = "std430";
		return " ";
	} break;
	case ShaderBaseType::RWBuffer: {
		*extra = "std430";
		return " ";
	} break;
	case ShaderBaseType::ROTexels: {
		*extra = "";
		return "textureBuffer";
	} break;
	case ShaderBaseType::RWTexels: {
		*extra = GetImageTexelFormat(type->image.texel.type, type->image.texel.size, type->image.texel.components);
		return "imageBuffer";
	} break;
	case ShaderBaseType::Input: {
		*extra = "";
		const string prefix =
			(type->image.texel.type == ShaderBaseType::Unsigned) ? "u" :
			(type->image.texel.type == ShaderBaseType::Signed) ? "i" : "";
		return prefix + "subpassInput";
	} break;
	}
	
	*extra = "";
	return "";
}

// ====================================================================================================================
string NameHelper::GetBindingTableName(const ShaderType* type)
{
	string basename{};
	switch (type->baseType)
	{
	case ShaderBaseType::Sampler: {
		const auto postfix = GetImageDimsPostfix(type->image.dims);
		basename = "sampler" + postfix;
	} break;
	case ShaderBaseType::Image: {
		const auto postfix = GetImageDimsPostfix(type->image.dims);
		const auto format = 
			GetImageTexelFormat(type->image.texel.type, type->image.texel.size, type->image.texel.components);
		basename = "image" + postfix + "_" + format;
	} break;
	case ShaderBaseType::Uniform:
	case ShaderBaseType::ROBuffer:
	case ShaderBaseType::RWBuffer: basename = "INVALID"; break;
	case ShaderBaseType::ROTexels: {
		basename = "rotexels";
	} break;
	case ShaderBaseType::RWTexels: {
		const auto format =
			GetImageTexelFormat(type->image.texel.type, type->image.texel.size, type->image.texel.components);
		basename = "rwtexels_" + format;
	} break;
	}

	// Create final name
	std::transform(basename.begin(), basename.end(), basename.begin(), ::toupper);
	return "_" + basename + "_TABLE_";
}

// ====================================================================================================================
string NameHelper::GetImageDimsPostfix(ImageDims dims)
{
	switch (dims)
	{
	case ImageDims::E1D: return "1D";
	case ImageDims::E2D: return "2D";
	case ImageDims::E3D: return "3D";
	case ImageDims::E1DArray: return "1DArray";
	case ImageDims::E2DArray: return "2DArray";
	case ImageDims::Cube: return "Cube";
	default: return "";
	}
}

// ====================================================================================================================
string NameHelper::GetImageTexelPrefix(ShaderBaseType type)
{
	switch (type)
	{
	case ShaderBaseType::Unsigned: return "u";
	case ShaderBaseType::Signed: return "i";
	default: return "";
	}
}

// ====================================================================================================================
string NameHelper::GetImageTexelFormat(ShaderBaseType type, uint8 size, uint8 dim0)
{
	switch (type)
	{
	case ShaderBaseType::Unsigned:
		return (dim0 == 1) ? "r32ui" : (dim0 == 2) ? "rg32ui" : (dim0 == 4) ? "rgba32ui" : "";
	case ShaderBaseType::Signed:
		return (dim0 == 1) ? "r32i" : (dim0 == 2) ? "rg32i" : (dim0 == 4) ? "rgba32i" : "";
	case ShaderBaseType::Float: {
		if (size == 4) {
			return (dim0 == 1) ? "r32f" : (dim0 == 2) ? "rg32f" : (dim0 == 4) ? "rgba32f" : "";
		}
		else if (size == 1) {
			return (dim0 == 1) ? "r8" : (dim0 == 2) ? "rg8" : (dim0 == 4) ? "rgba8" : "";
		}
		return "";
	}
	default: return "";
	}
}

// ====================================================================================================================
string NameHelper::GetBindingIndexText(uint32 index)
{
	return ((index & 1) == 0)
		? mkstr("(_bidx_.index%u & 0x0000FFFF)", index / 2)
		: mkstr("(_bidx_.index%u >> 16)", index / 2);
}

// ====================================================================================================================
string NameHelper::GetBuiltinName(const string& vslName)
{
	static const std::unordered_map<string, string> BUILTIN_MAP {
		{ "$VertexIndex", "gl_VertexIndex" }, { "$InstanceIndex", "gl_InstanceIndex" },
		{ "$DrawIndex", "gl_DrawID" }, { "$VertexBase", "gl_BaseVertex" }, { "$InstanceBase", "gl_BaseInstance" },

		{ "$Position", "gl_Position" }, { "$PointSize", "gl_PointSize" },

		{ "$FragCoord", "gl_FragCoord" }, { "$FrontFacing", "gl_FrontFacing" }, { "$PointCoord", "gl_PointCoord" }, 
		{ "$PrimitiveID", "gl_PrimitiveID" }
	};

	const auto it = BUILTIN_MAP.find(vslName);
	if (it != BUILTIN_MAP.end()) {
		return it->second;
	}
	return "";
}

} // namespace vsl
