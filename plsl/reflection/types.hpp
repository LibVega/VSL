/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#pragma once

#include <plsl/config.hpp>

#include <vector>

#define PLSL_MAX_ARRAY_SIZE (255)   // The maximum size of an array for struct members
#define PLSL_MAX_MEMBER_COUNT (32)  // The maximum number of struct members


namespace plsl
{

// Enum of the base shader types
enum class ShaderBaseType : uint8
{
	Void,          // Special type for errors and function returns
	Boolean,       // Boolean scalar/vector
	Integer,       // Integer scalar/vector
	Float,         // Floating point scalar/vector/matrix
	Sampler,       // Vk sampler, glsl 'sampler'
	BoundSampler,  // Vk combined image/sampler, glsl 'sampler*D'
	Texture,       // Vk sampled image, glsl `texture*D`
	Image,         // Vk storage image, glsl `image*D`
	UBuffer,       // Vk uniform buffer, glsl `uniform <name> { ... }`
	ROBuffer,      // Vk readonly storage buffer, glsl `readonly buffer <name> { ... }`
	RWBuffer,      // Vk read/write storage buffer, glsl `buffer <name> { ... }`
	ROTexels,      // Vk uniform texel buffer, glsl `samplerBuffer`
	WOTexels,      // Vk storage texel buffer, glsl `writeonly imageBuffer`
	Input,         // Vk input attachment, glsl `subpassInput`
	Struct,        // Compound struct type, glsl `struct <name> { ... }`
}; // enum class ShaderBaseType


// The different dimensionalities that BoundSampler/Texture/Image can take on
enum class ImageDims : uint8
{
	E1D,       // Single 1D texture
	E2D,       // Single 2D texture
	E3D,       // Single 3D texture
	E1DArray,  // Array of 1D textures
	E2DArray,  // Array of 2D textures
	Cube,      // Single cubemap texture
	CubeArray  // Array of cubemap textures
}; // enum class ImageDims


// Contains information about a struct member (similar to ShaderType)
struct StructMemberType final
{
	ShaderBaseType baseType;  // The base type for the member
	uint8 size;               // The type size (in bytes)
	uint8 dims[2];            // The type dimensions ([0] = vec dims, [1] = matrix columns)
	uint8 arraySize;          // The array element count, or 1 if not an array (array max size is 255)
}; // struct StructMemberType


// Contains information about the type of an object, variable, or operand within a shader 
struct ShaderType final
{
public:
	ShaderBaseType baseType;  // The base type
	union {
		struct ImageInfo
		{
			ImageDims dims;   // The dimensions of the image type
		} image;
		struct NumericInfo
		{
			uint8 size;       // The size of the type (in bytes) (byte/short/int/long/float/double selection)
			uint8 dims[2];    // The dimensions of the type ([0] = vec dims, [1] = matrix columns)
		} numeric;
	};
	struct StructInfo
	{
		std::vector<StructMemberType> members; // For user struct types, this is the struct members
	} userStruct;
}; // struct ShaderType

} // namespace plsl
