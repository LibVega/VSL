/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./Types.hpp"

#include <algorithm>
#include <cctype>


namespace vsl
{

// ====================================================================================================================
string TexelRankGetSuffix(TexelRank rank)
{
	switch (rank)
	{
	case TexelRank::E1D: return "1D";
	case TexelRank::E2D: return "2D";
	case TexelRank::E3D: return "3D";
	case TexelRank::E1DArray: return "1DArray";
	case TexelRank::E2DArray: return "2DArray";
	case TexelRank::Cube: return "Cube";
	case TexelRank::Buffer: return "Buffer";
	default: return "UNKNOWN_RANK";
	}
}

// ====================================================================================================================
uint32 TexelRankGetComponentCount(TexelRank rank)
{
	switch (rank)
	{
	case TexelRank::E1D: return 1;
	case TexelRank::E2D: return 2;
	case TexelRank::E3D: return 3;
	case TexelRank::E1DArray: return 2;
	case TexelRank::E2DArray: return 3;
	case TexelRank::Cube: return 2;
	case TexelRank::Buffer: return 1;
	default: return 0;
	}
}


// ====================================================================================================================
// ====================================================================================================================
StructType::StructType(const string& name, const std::vector<Member>& members)
	: name_{ name }
	, members_{ members }
	, offsets_{ }
	, size_{ 0 }
	, alignment_{ 0 }
{
	if (!members.empty()) {
		for (const auto& mem : members_) {
			// Align
			const auto align = mem.type->numeric.size;
			if ((size_ % align) != 0) {
				size_ += (align - (size_ % align));
			}
			if (align > alignment_) {
				alignment_ = align;
			}

			// Add member
			offsets_.push_back(size_);
			size_ += (mem.type->numeric.size * mem.type->numeric.dims[0] * mem.type->numeric.dims[1] * mem.arraySize);
		}

		// Final size alignment
		if ((size_ % alignment_) != 0) {
			size_ += (alignment_ - (size_ % alignment_));
		}
	}
}

// ====================================================================================================================
const StructType::Member* StructType::getMember(const string& name, uint32* offset) const
{
	for (uint32 i = 0; i < members_.size(); ++i) {
		if (members_[i].name != name) {
			continue;
		}
		if (offset) {
			*offset = offsets_[i];
		}
		return &(members_[i]);
	}

	if (offset) {
		*offset = 0;
	}
	return nullptr;
}


// ====================================================================================================================
// ====================================================================================================================
bool TexelFormat::isSame(const TexelFormat* format) const
{
	return (type == format->type) && (size == format->size) && (count == format->count);
}

// ====================================================================================================================
string TexelFormat::getVSLName() const
{
	// Base name
	string base{ "BAD_TYPE" };
	switch (type)
	{
	case TexelType::Signed: base = (size == 4) ? "int" : "BAD_SIZE"; break;
	case TexelType::Unsigned: base = (size == 4) ? "uint" : "BAD_SIZE"; break;
	case TexelType::Float: base = (size == 4) ? "float" : "BAD_SIZE"; break;
	case TexelType::UNorm: base = (size == 1) ? "u8norm" : (size == 2) ? "u16norm" : "BAD_SIZE"; break;
	case TexelType::SNorm: base = (size == 1) ? "s8norm" : (size == 2) ? "s16norm" : "BAD_SIZE"; break;
	}

	// Count
	string countStr{ };
	switch (count)
	{
	case 2: countStr = "2"; break;
	case 4: countStr = "4"; break;
	}
	
	// Combine
	return base + countStr;
}

// ====================================================================================================================
string TexelFormat::getGLSLName() const
{
	// Base text
	string base{ "BAD_COUNT" };
	switch (count)
	{
	case 1: base = "r"; break;
	case 2: base = "rg"; break;
	case 4: base = "rgba"; break;
	}

	// Type
	string typeStr{ "BAD_TYPE" };
	switch (type)
	{
	case TexelType::Signed: typeStr = (size == 4) ? "32i" : "BAD_SIZE"; break;
	case TexelType::Unsigned: typeStr = (size == 4) ? "32ui" : "BAD_SIZE"; break;
	case TexelType::Float: typeStr = (size == 4) ? "32f" : "BAD_SIZE"; break;
	case TexelType::UNorm: typeStr = (size == 1) ? "8" : (size == 1) ? "16" : "BAD_SIZE"; break;
	case TexelType::SNorm: typeStr = (size == 1) ? "8_snorm" : (size == 1) ? "16_snorm" : "BAD_SIZE"; break;
	}

	// Combine
	return base + typeStr;
}

// ====================================================================================================================
string TexelFormat::getVSLPrefix() const
{
	return (type == TexelType::Signed) ? "I" : (type == TexelType::Unsigned) ? "U" : "";
}

// ====================================================================================================================
string TexelFormat::getGLSLPrefix() const
{
	return (type == TexelType::Signed) ? "i" : (type == TexelType::Unsigned) ? "u" : "";
}

// ====================================================================================================================
const ShaderType* TexelFormat::asDataType() const
{
	const auto& bit = TypeList::BuiltinTypes();
	if (isFloatingType()) {
		return &bit.at((count == 1) ? "float" : (count == 2) ? "float2" : "float4");
	}
	else if (isSigned()) {
		return &bit.at((count == 1) ? "int" : (count == 2) ? "int2" : "int4");
	}
	else { // isUnsigned()
		return &bit.at((count == 1) ? "uint" : (count == 2) ? "uint2" : "uint4");
	}
}


// ====================================================================================================================
// ====================================================================================================================
bool ShaderType::isSame(const ShaderType* otherType) const
{
	// Base type check
	if (baseType != otherType->baseType) {
		return false;
	}

	// Type-specific checks
	switch (baseType)
	{
	case BaseType::Void: return true;
	case BaseType::Boolean:
	case BaseType::Signed:
	case BaseType::Unsigned:
	case BaseType::Float:
		return (numeric.size == otherType->numeric.size) && (numeric.dims[0] == otherType->numeric.dims[0]) &&
			(numeric.dims[1] == otherType->numeric.dims[1]);
	case BaseType::Sampler:
	case BaseType::Image: return (texel.rank == otherType->texel.rank) && texel.format->isSame(otherType->texel.format);
	case BaseType::ROBuffer:
	case BaseType::RWBuffer: return (buffer.structType == otherType->buffer.structType);
	case BaseType::ROTexels:
	case BaseType::RWTexels: 
	case BaseType::SPInput: return texel.format->isSame(otherType->texel.format);
	case BaseType::Uniform: return (buffer.structType == otherType->buffer.structType);
	case BaseType::Struct: return (userStruct.type->name() == otherType->userStruct.type->name());
	default: return false;
	}
}

// ====================================================================================================================
bool ShaderType::hasImplicitCast(const ShaderType* targetType) const
{
	// Check for same-ness
	if ((this == targetType) || isSame(targetType)) {
		return true;
	}

	// Only numerics can cast
	if (!isNumericType() || !targetType->isNumericType()) {
		return false;
	}

	// Must be same dimensions
	if ((numeric.dims[0] != targetType->numeric.dims[0]) || (numeric.dims[1] != targetType->numeric.dims[1])) {
		return false;
	}

	// Anything can cast to float at the same or greater size
	if (targetType->baseType == BaseType::Float) {
		return (numeric.size <= targetType->numeric.size);
	}
	if (baseType == BaseType::Float) {
		return false; // Cannot go from float -> int
	}

	// Signed/Unsigned -> Unsigned for bigger or same size works
	if (targetType->baseType == BaseType::Unsigned) {
		return (numeric.size <= targetType->numeric.size);
	}
	if (baseType == BaseType::Unsigned) {
		return false; // Cannot go from uint -> int
	}

	// Self-cast, bigger or same size
	return (numeric.size <= targetType->numeric.size);
}

// ====================================================================================================================
string ShaderType::getVSLName() const
{
	static const auto prefix = [](const ShaderType* texelType) -> string {
		return string(
			(texelType->baseType == BaseType::Signed) ? "I" :
			(texelType->baseType == BaseType::Unsigned) ? "U" : "");
	};

	switch (baseType)
	{
	case BaseType::Void: return "void";
	case BaseType::Boolean: return
		(numeric.dims[0] == 1) ? "bool" : (numeric.dims[0] == 2) ? "bool2" :
		(numeric.dims[0] == 3) ? "bool3" : "bool4";
	case BaseType::Signed: return (numeric.size != 4) ? "BAD_SIGNED_TYPE_SIZE" :
		(numeric.dims[0] == 1) ? "int" : (numeric.dims[0] == 2) ? "int2" :
		(numeric.dims[0] == 3) ? "int3" : "int4";
	case BaseType::Unsigned: return (numeric.size != 4) ? "BAD_UNSIGNED_TYPE_SIZE" :
		(numeric.dims[0] == 1) ? "uint" : (numeric.dims[0] == 2) ? "uint2" :
		(numeric.dims[0] == 3) ? "uint3" : "uint4";
	case BaseType::Float: {
		if (numeric.size != 4) {
			return "BAD_FLOAT_TYPE_SIZE";
		}
		switch (numeric.dims[1])
		{
		case 1: return
			(numeric.dims[0] == 1) ? "float" : (numeric.dims[0] == 2) ? "float2" :
			(numeric.dims[0] == 3) ? "float3" : "float4";
		case 2: return (numeric.dims[0] == 2) ? "float2x2" : (numeric.dims[0] == 3) ? "float2x3" : "float2x4";
		case 3: return (numeric.dims[0] == 2) ? "float3x2" : (numeric.dims[0] == 3) ? "float3x3" : "float3x4";
		default: return (numeric.dims[0] == 2) ? "float4x2" : (numeric.dims[0] == 3) ? "float4x3" : "float4x4";
		}
	} break;
	case BaseType::Sampler: return texel.format->getVSLPrefix() + "Sampler" + TexelRankGetSuffix(texel.rank);
	case BaseType::Image: return texel.format->getVSLPrefix() + "Image" + TexelRankGetSuffix(texel.rank);
	case BaseType::ROBuffer: return "ROBuffer<" + buffer.structType->userStruct.type->name() + ">";
	case BaseType::RWBuffer: return "RWBuffer<" + buffer.structType->userStruct.type->name() + ">";
	case BaseType::ROTexels: return "RO" + texel.format->getVSLPrefix() + "Texels";
	case BaseType::RWTexels: return "RWTexels<" + texel.format->getVSLName() + ">";
	case BaseType::SPInput: return texel.format->getVSLName();
	case BaseType::Uniform: return buffer.structType->userStruct.type->name();
	case BaseType::Struct: return buffer.structType->userStruct.type->name();
	default: return "INVALID_TYPE";
	}
}

// ====================================================================================================================
string ShaderType::getGLSLName() const
{
	static const auto prefix = [](const ShaderType* texelType) -> string {
		return string(
			(texelType->baseType == BaseType::Signed) ? "i" :
			(texelType->baseType == BaseType::Unsigned) ? "u" : "");
	};

	switch (baseType)
	{
	case BaseType::Void: return "void";
	case BaseType::Boolean: return
		(numeric.dims[0] == 1) ? "bool" : (numeric.dims[0] == 2) ? "bvec2" :
		(numeric.dims[0] == 3) ? "bvec3" : "bvec4";
	case BaseType::Signed: return (numeric.size != 4) ? "BAD_SIGNED_TYPE_SIZE" :
		(numeric.dims[0] == 1) ? "int" : (numeric.dims[0] == 2) ? "ivec2" :
		(numeric.dims[0] == 3) ? "ivec3" : "ivec4";
	case BaseType::Unsigned: return (numeric.size != 4) ? "BAD_UNSIGNED_TYPE_SIZE" :
		(numeric.dims[0] == 1) ? "uint" : (numeric.dims[0] == 2) ? "uvec2" :
		(numeric.dims[0] == 3) ? "uvec3" : "uvec4";
	case BaseType::Float: {
		if (numeric.size != 4) {
			return "BAD_FLOAT_TYPE_SIZE";
		}
		switch (numeric.dims[1])
		{
		case 1: return
			(numeric.dims[0] == 1) ? "float" : (numeric.dims[0] == 2) ? "vec2" :
			(numeric.dims[0] == 3) ? "vec3" : "vec4";
		case 2: return (numeric.dims[0] == 2) ? "mat2x2" : (numeric.dims[0] == 3) ? "mat2x3" : "mat2x4";
		case 3: return (numeric.dims[0] == 2) ? "mat3x2" : (numeric.dims[0] == 3) ? "mat3x3" : "mat3x4";
		default: return (numeric.dims[0] == 2) ? "mat4x2" : (numeric.dims[0] == 3) ? "mat4x3" : "mat4x4";
		}
	} break;
	case BaseType::Sampler: return texel.format->getGLSLPrefix() + "sampler" + TexelRankGetSuffix(texel.rank);
	case BaseType::Image: return texel.format->getGLSLPrefix() + "image" + TexelRankGetSuffix(texel.rank);
	case BaseType::ROBuffer:
	case BaseType::RWBuffer: return buffer.structType->userStruct.type->name() + "_t";
	case BaseType::ROTexels: return texel.format->getGLSLPrefix() + "textureBuffer";
	case BaseType::RWTexels: return texel.format->getGLSLPrefix() + "imageBuffer";
	case BaseType::SPInput: return texel.format->getGLSLPrefix() + "subpassInput";
	case BaseType::Uniform: return buffer.structType->userStruct.type->name() + "_t";
	case BaseType::Struct: return buffer.structType->userStruct.type->name() + "_t";
	default: return "INVALID_TYPE";
	}
}

// ====================================================================================================================
uint32 ShaderType::getBindingCount() const
{
	if (!isNumericType()) {
		return 0;
	}

	const uint32 slotCount = ((numeric.size * numeric.dims[0]) > 16) ? 2u : 1u;
	return slotCount * numeric.dims[1];
}


// ====================================================================================================================
// ====================================================================================================================
const ShaderType* TypeList::addType(const string& name, const ShaderType& type)
{
	auto it = types_.find(name);
	if (it == types_.end()) {
		return &(types_[name] = type);
	}
	error_ = mkstr("type name '%s' already exists", name.c_str());
	return nullptr;
}

// ====================================================================================================================
const ShaderType* TypeList::getType(const string& name) const
{
	auto it = types_.find(name);
	if (it != types_.end()) {
		return &(it->second);
	}
	it = BuiltinTypes_.find(name);
	if (it != BuiltinTypes_.end()) {
		return &(it->second);
	}
	error_ = mkstr("no type with name '%s' found", name.c_str());
	return nullptr;
}

// ====================================================================================================================
const StructType* TypeList::addStructType(const string& name, const StructType& type)
{
	auto it = structs_.find(name);
	if (it == structs_.end()) {
		return &(structs_[name] = type);
	}
	error_ = mkstr("struct type '%s' already exists", name.c_str());
	return nullptr;
}

// ====================================================================================================================
const StructType* TypeList::getStructType(const string& name) const
{
	auto it = structs_.find(name);
	if (it != structs_.end()) {
		return &(it->second);
	}
	error_ = mkstr("no struct type with name '%s' found", name.c_str());
	return nullptr;
}

// ====================================================================================================================
const ShaderType* TypeList::parseOrGetType(const string& name)
{
	// Normalize
	auto typeName = name;
	typeName.erase(std::remove_if(typeName.begin(), typeName.end(), ::isspace), typeName.end());

	// Check if it exists already
	if (const auto type = getType(typeName); type) {
		return type;
	}

	// Get the subtype (if no subtype, return error because all non-subtype types are already known)
	const auto stIndex = typeName.find('<');
	if (stIndex == string::npos) {
		error_ = mkstr("unknown type '%s'", typeName.c_str());
		return nullptr;
	}
	auto genType = ParseGenericType(typeName.substr(0, stIndex));
	if (!genType) {
		error_ = mkstr("unknown generic type '%s'", typeName.c_str());
		return nullptr;
	}
	auto type = *genType;

	// Parse subtype
	const auto subtype = typeName.substr(stIndex + 1, typeName.length() - stIndex - 2);
	if ((type.baseType == BaseType::Image) || (type.baseType == BaseType::RWTexels)) {
		const auto format = GetTexelFormat(subtype);
		if (!format) {
			error_ = mkstr("invalid texel format '%s'", subtype.c_str());
			return nullptr;
		}
		type.texel.format = format;
	}
	else { // Buffer types
		const auto structType = getType(subtype);
		if (!structType || !structType->isStruct()) {
			error_ = mkstr("no struct type '%s' found", subtype.c_str());
			return nullptr;
		}
		type.buffer.structType = structType;
	}

	// Add and return the type
	return addType(typeName, type);
}

// ====================================================================================================================
const ShaderType* TypeList::GetBuiltinType(const string& name)
{
	if (BuiltinTypes_.empty()) {
		Initialize();
	}

	const auto it = BuiltinTypes_.find(name);
	if (it != BuiltinTypes_.end()) {
		return &(it->second);
	}
	return nullptr;
}

// ====================================================================================================================
const TexelFormat* TypeList::GetTexelFormat(const string& format)
{
	auto it = Formats_.find(format);
	if (it != Formats_.end()) {
		return &(it->second);
	}
	return nullptr;
}

// ====================================================================================================================
const ShaderType* TypeList::GetNumericType(BaseType baseType, uint32 size, uint32 dim0, uint32 dim1)
{
	if (BuiltinTypes_.empty()) {
		Initialize();
	}

	for (const auto& pair : BuiltinTypes_) {
		const auto& type = pair.second;
		if ((type.baseType == baseType) && (type.numeric.size == size) && (type.numeric.dims[0] == dim0) &&
				(type.numeric.dims[1] == dim1)) {
			return &type;
		}
	}
	return nullptr;
}

// ====================================================================================================================
const ShaderType* TypeList::ParseGenericType(const string& baseType)
{
	static const auto E1D = TexelRankGetSuffix(TexelRank::E1D);
	static const auto E2D = TexelRankGetSuffix(TexelRank::E2D);
	static const auto E3D = TexelRankGetSuffix(TexelRank::E3D);
	static const auto E1DARRAY = TexelRankGetSuffix(TexelRank::E1DArray);
	static const auto E2DARRAY = TexelRankGetSuffix(TexelRank::E2DArray);
	static const auto CUBE = TexelRankGetSuffix(TexelRank::Cube);

	// Check cache
	const auto it = GenericTypes_.find(baseType);
	if (it != GenericTypes_.end()) {
		return &(it->second);
	}

	// Create new generic type
	ShaderType genType{};
	if (baseType.find("Image") == 0) {
		auto rank = baseType.substr(5);
		if (rank == E1D) {
			genType = { BaseType::Image, TexelRank::E1D, nullptr };
		}
		else if (rank == E2D) {
			genType = { BaseType::Image, TexelRank::E2D, nullptr };
		}
		else if (rank == E3D) {
			genType = { BaseType::Image, TexelRank::E3D, nullptr };
		}
		else if (rank == E1DARRAY) {
			genType = { BaseType::Image, TexelRank::E1DArray, nullptr };
		}
		else if (rank == E2DARRAY) {
			genType = { BaseType::Image, TexelRank::E2DArray, nullptr };
		}
		else if (rank == CUBE) {
			genType = { BaseType::Image, TexelRank::Cube, nullptr };
		}
		else {
			return nullptr;
		}
	}
	else if (baseType == "ROBuffer") {
		genType = { BaseType::ROBuffer, nullptr };
	}
	else if (baseType == "RWBuffer") {
		genType = { BaseType::RWBuffer, nullptr };
	}
	else if (baseType == "RWTexels") {
		genType = { BaseType::RWTexels, TexelRank::Buffer, nullptr };
	}
	else {
		return nullptr;
	}

	// Add and return
	return &(GenericTypes_[baseType] = genType);
}

// ====================================================================================================================
void TypeList::Initialize()
{
	if (Initialized_) {
		return;
	}
	Initialized_ = true;

	const TexelFormat* float4 = &(Formats_["float4"]);
	const TexelFormat* int4 = &(Formats_["int4"]);
	const TexelFormat* uint4 = &(Formats_["uint4"]);

	// Samplers
	BuiltinTypes_["Sampler1D"]       = { BaseType::Sampler, TexelRank::E1D, float4 };
	BuiltinTypes_["Sampler2D"]       = { BaseType::Sampler, TexelRank::E2D, float4 };
	BuiltinTypes_["Sampler3D"]       = { BaseType::Sampler, TexelRank::E3D, float4 };
	BuiltinTypes_["Sampler1DArray"]  = { BaseType::Sampler, TexelRank::E1DArray, float4 };
	BuiltinTypes_["Sampler2DArray"]  = { BaseType::Sampler, TexelRank::E2DArray, float4 };
	BuiltinTypes_["SamplerCube"]     = { BaseType::Sampler, TexelRank::Cube, float4 };
	BuiltinTypes_["ISampler1D"]      = { BaseType::Sampler, TexelRank::E1D, int4 };
	BuiltinTypes_["ISampler2D"]      = { BaseType::Sampler, TexelRank::E2D, int4 };
	BuiltinTypes_["ISampler3D"]      = { BaseType::Sampler, TexelRank::E3D, int4 };
	BuiltinTypes_["ISampler1DArray"] = { BaseType::Sampler, TexelRank::E1DArray, int4 };
	BuiltinTypes_["ISampler2DArray"] = { BaseType::Sampler, TexelRank::E2DArray, int4 };
	BuiltinTypes_["ISamplerCube"]    = { BaseType::Sampler, TexelRank::Cube, int4 };
	BuiltinTypes_["USampler1D"]      = { BaseType::Sampler, TexelRank::E1D, uint4 };
	BuiltinTypes_["USampler2D"]      = { BaseType::Sampler, TexelRank::E2D, uint4 };
	BuiltinTypes_["USampler3D"]      = { BaseType::Sampler, TexelRank::E3D, uint4 };
	BuiltinTypes_["USampler1DArray"] = { BaseType::Sampler, TexelRank::E1DArray, uint4 };
	BuiltinTypes_["USampler2DArray"] = { BaseType::Sampler, TexelRank::E2DArray, uint4 };
	BuiltinTypes_["USamplerCube"]    = { BaseType::Sampler, TexelRank::Cube, uint4 };

	// ROTexels
	BuiltinTypes_["ROTexels"]  = { BaseType::ROTexels, TexelRank::Buffer, float4 };
	BuiltinTypes_["ROITexels"] = { BaseType::ROTexels, TexelRank::Buffer, int4 };
	BuiltinTypes_["ROUTexels"] = { BaseType::ROTexels, TexelRank::Buffer, uint4 };
}

// ====================================================================================================================
bool TypeList::Initialized_{ false };
TypeList::TypeMap TypeList::BuiltinTypes_ {
	{ "void", { } },
	// Boolean
	{ "bool",  { BaseType::Boolean, 4, 1, 1 } }, { "bool2", { BaseType::Boolean, 4, 2, 1 } },
	{ "bool3", { BaseType::Boolean, 4, 3, 1 } }, { "bool4", { BaseType::Boolean, 4, 4, 1 } },
	// Integer
	{ "int",   { BaseType::Signed, 4, 1, 1 } }, { "int2",  { BaseType::Signed, 4, 2, 1 } },
	{ "int3",  { BaseType::Signed, 4, 3, 1 } }, { "int4",  { BaseType::Signed, 4, 4, 1 } },
	{ "uint",  { BaseType::Unsigned, 4, 1, 1 } }, { "uint2", { BaseType::Unsigned, 4, 2, 1 } },
	{ "uint3", { BaseType::Unsigned, 4, 3, 1 } }, { "uint4", { BaseType::Unsigned, 4, 4, 1 } },
	// Float
	{ "float",  { BaseType::Float, 4, 1, 1 } }, { "float2", { BaseType::Float, 4, 2, 1 } },
	{ "float3", { BaseType::Float, 4, 3, 1 } }, { "float4", { BaseType::Float, 4, 4, 1 } },
	// Matrices
	{ "float2x2", { BaseType::Float, 4, 2, 2 } },
	{ "float3x3", { BaseType::Float, 4, 3, 3 } },
	{ "float4x4", { BaseType::Float, 4, 4, 4 } },
	{ "float2x3", { BaseType::Float, 4, 3, 2 } }, { "float3x2", { BaseType::Float, 4, 2, 3 } },
	{ "float2x4", { BaseType::Float, 4, 4, 2 } }, { "float4x2", { BaseType::Float, 4, 2, 4 } },
	{ "float3x4", { BaseType::Float, 4, 4, 3 } }, { "float4x3", { BaseType::Float, 4, 3, 4 } },
};
TypeList::FormatMap TypeList::Formats_ {
	// Signed
	{ "int", { TexelType::Signed, 4, 1 } }, { "int2", { TexelType::Signed, 4, 2 } },
	{ "int4", { TexelType::Signed, 4, 4 } },
	// Unsigned
	{ "uint", { TexelType::Unsigned, 4, 1 } }, { "uint2", { TexelType::Unsigned, 4, 2 } },
	{ "uint4", { TexelType::Unsigned, 4, 4 } },
	// Float
	{ "float", { TexelType::Float, 4, 1 } }, { "float2", { TexelType::Float, 4, 2 } },
	{ "float4", { TexelType::Float, 4, 4 } },
	// UNorm
	{ "u8norm", { TexelType::UNorm, 1, 1 } }, { "u8norm2", { TexelType::UNorm, 1, 2 } },
	{ "u8norm4", { TexelType::UNorm, 1, 4 } },
	{ "u16norm", { TexelType::UNorm, 2, 1 } }, { "u16norm2", { TexelType::UNorm, 2, 2 } },
	{ "u16norm4", { TexelType::UNorm, 2, 4 } },
	// SNorm
	{ "s8norm", { TexelType::SNorm, 1, 1 } }, { "s8norm2", { TexelType::SNorm, 1, 2 } },
	{ "s8norm4", { TexelType::SNorm, 1, 4 } },
	{ "s16norm", { TexelType::SNorm, 2, 1 } }, { "s16norm2", { TexelType::SNorm, 2, 2 } },
	{ "s16norm4", { TexelType::SNorm, 2, 4 } },
};
TypeList::TypeMap TypeList::GenericTypes_{ };

} // namespace vsl
