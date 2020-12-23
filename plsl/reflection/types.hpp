/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#pragma once

#include <plsl/config.hpp>

#include <vector>

#define PLSL_MAX_ARRAY_SIZE (255u)   // The maximum size of an array for struct members
#define PLSL_MAX_MEMBER_COUNT (32u)  // The maximum number of struct members


namespace plsl
{

// Enum of the base shader types
enum class ShaderBaseType : uint8
{
	Void,          // Special type for errors and function returns
	Boolean,       // Boolean scalar/vector
	SInteger,      // Signed integer scalar/vector
	UInteger,      // Unsigned integer scalar/vector
	Float,         // Floating point scalar/vector/matrix
	Sampler,       // Vk sampler, glsl 'sampler'
	BoundSampler,  // Vk combined image/sampler, glsl '[ ui]sampler*D'
	Texture,       // Vk sampled image, glsl `[ ui]texture*D`
	Image,         // Vk storage image, glsl `writeonly image*D` w/o layout
	Uniform,       // Vk uniform buffer, glsl `uniform <name> { ... }`
	ROBuffer,      // Vk readonly storage buffer, glsl `readonly buffer <name> { ... }`
	RWBuffer,      // Vk read/write storage buffer, glsl `buffer <name> { ... }`
	ROTexels,      // Vk uniform texel buffer, glsl `samplerBuffer`
	RWTexels,      // Vk storage texel buffer, glsl `writeonly imageBuffer` w/o layout
	Input,         // Vk input attachment, glsl `[ ui]subpassInput`
	Struct,        // User-defined POD struct
}; // enum class ShaderBaseType


// The different dimensionalities that BoundSampler/Texture/Image can take on
enum class ImageDims : uint8
{
	None,       // Un-dimensioned texture
	E1D,        // Single 1D texture
	E2D,        // Single 2D texture
	E3D,        // Single 3D texture
	E1DArray,   // Array of 1D textures
	E2DArray,   // Array of 2D textures
	Cube,       // Single cubemap texture
	CubeArray,  // Array of cubemap textures
	Shadow,     // Sampler-specific type for shadows
}; // enum class ImageDims


// Contains information about a struct member (similar to ShaderType)
struct StructMember final
{
	string name;              // The struct member name
	ShaderBaseType baseType;  // The base type for the member
	uint8 size;               // The type size (in bytes)
	uint8 dims[2];            // The type dimensions ([0] = vec dims, [1] = matrix columns)
	uint8 arraySize;          // The array element count, or 1 if not an array (array max size is 255)

	inline bool operator == (const StructMember& other) const {
		return (name == other.name) && (baseType == other.baseType) && (size == other.size) &&
			(dims[0] == other.dims[0]) && (dims[1] == other.dims[1]) && (arraySize == other.arraySize);
	}
	inline bool operator != (const StructMember& other) const {
		return (name != other.name) || (baseType != other.baseType) || (size != other.size) ||
			(dims[0] != other.dims[0]) || (dims[1] != other.dims[1]) || (arraySize != other.arraySize);
	}
}; // struct StructMember


// Contains information about the type of an object, variable, or operand within a shader
struct ShaderType final
{
public:
	ShaderType()
		: baseType{ ShaderBaseType::Void }, image{}, numeric{}, buffer{}, userStruct{}
	{ }
	ShaderType(ShaderBaseType baseType, ImageDims dims)
		: baseType{ baseType }, image{ dims, { } }, numeric{}, buffer{}, userStruct{}
	{ }
	ShaderType(ShaderBaseType baseType, ImageDims dims, ShaderBaseType compType, uint8 count)
		: baseType{ baseType }, image{ dims, { compType, 4, count } }, numeric{}, buffer{}, userStruct{}
	{ }
	ShaderType(ShaderBaseType baseType, uint8 size, uint8 components, uint8 columns = 0)
		: baseType{ baseType }, image{}, numeric{ size, { components, columns } }, buffer{}, userStruct{}
	{ }
	ShaderType(ShaderBaseType baseType, const string& bufferType)
		: baseType{ baseType }, image{}, numeric{}, buffer{ bufferType }, userStruct{}
	{ }
	ShaderType(const string& structName, const std::vector<StructMember>& members = {})
		: baseType{ ShaderBaseType::Struct }, image{}, numeric{}, buffer{}, userStruct{ structName, members }
	{ }
	~ShaderType() { }

	inline bool isVoid() const { return (baseType == ShaderBaseType::Void); }
	inline bool isNumeric() const {
		return (baseType == ShaderBaseType::Float) || (baseType == ShaderBaseType::SInteger) ||
			(baseType == ShaderBaseType::UInteger);
	}
	inline bool isSampler() const {
		return (baseType == ShaderBaseType::Sampler) || (baseType == ShaderBaseType::BoundSampler);
	}
	inline bool isTexture() const { return (baseType == ShaderBaseType::Texture); }
	inline bool isImage() const { return (baseType == ShaderBaseType::Image); }
	inline bool isBuffer() const {
		return (baseType == ShaderBaseType::Uniform) || (baseType == ShaderBaseType::ROBuffer) ||
			(baseType == ShaderBaseType::RWBuffer) || (baseType == ShaderBaseType::ROTexels) ||
			(baseType == ShaderBaseType::RWTexels);
	}
	inline bool isStruct() const { return (baseType == ShaderBaseType::Struct); }

	bool hasMember(const string& memberName) const;

public:
	ShaderBaseType baseType;      // The base type
	struct ImageInfo
	{
		ImageDims dims;           // The dimensions of the image type
		struct TexelInfo
		{
			ShaderBaseType type;  // The texel component type
			uint8 size;           // The texel component size
			uint8 components;     // The texel component count
		} texel;
	} image;
	struct NumericInfo
	{
		uint8 size;               // The size of the type (in bytes) (byte/short/int/long/float/double selection)
		uint8 dims[2];            // The dimensions of the type ([0] = vec dims, [1] = matrix columns)
	} numeric;
	struct BufferInfo
	{
		string structName;        // The name of the struct type that the buffer is populated with
	} buffer;
	struct StructInfo
	{
		string structName;                      // The name of the struct type
		std::vector<StructMember> members;  // The struct members
	} userStruct;
}; // struct ShaderType

} // namespace plsl
