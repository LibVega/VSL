/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#pragma once

#include "./Config.hpp"

#include <unordered_map>
#include <vector>


namespace vsl
{

// Enum of the base shader types
enum class BaseType : uint32
{
	Void     =  0,  // Special type for errors and function returns
	Boolean  =  1,  // Boolean scalar/vector
	Signed   =  2,  // Signed integer scalar/vector
	Unsigned =  3,  // Unsigned integer scalar/vector
	Float    =  4,  // Floating point scalar/vector/matrix
	Sampler  =  5,  // Vk combined image/sampler, glsl 'sampler*D' (no separate image and sampler objects)
	Image    =  6,  // Vk storage image, glsl `image*D` w/ layout
	ROBuffer =  7,  // Vk readonly storage buffer, glsl `readonly buffer <name> { ... }`
	RWBuffer =  8,  // Vk read/write storage buffer, glsl `buffer <name> { ... }`
	ROTexels =  9,  // Vk uniform texel buffer, glsl `textureBuffer`
	RWTexels = 10,  // Vk storage texel buffer, glsl `imageBuffer` w/ layout
	SPInput  = 11,  // Vk input attachment, glsl `[ ui]subpassInput`
	Uniform  = 12,  // Vk uniform buffer, glsl `uniform <name> { ... }`
	Struct   = 13   // User-defined POD struct
}; // enum class BaseType


struct ShaderType;
// Describes a user-defined struct type
struct StructType final
{
public:
	struct Member final
	{
		string name;
		uint32 arraySize;
		const ShaderType* type;
	}; // struct Member

	StructType(const string& name, const std::vector<Member>& members);
	StructType() : StructType("INVALID", {}) { }
	~StructType() { }

	/* Fields */
	inline const string& name() const { return name_; }
	inline const std::vector<Member>& members() const { return members_; }
	inline const std::vector<uint32>& offsets() const { return offsets_; }
	inline uint32 size() const { return size_; }
	inline uint32 alignment() const { return alignment_; }

	/* Member Access */
	const Member* getMember(const string& name, uint32* offset = nullptr) const;
	inline bool hasMember(const string& name) const { return !!getMember(name, nullptr); }

private:
	string name_;
	std::vector<Member> members_;
	std::vector<uint32> offsets_;
	uint32 size_;
	uint32 alignment_;
}; // struct StructType


// The different ranks (dimension counts) that texel-like objects can have
enum class TexelRank : uint32
{
	E1D = 0,  // Single 1D texture
	E2D = 1,  // Single 2D texture
	E3D = 2,  // Single 3D texture
	E1DArray = 3,  // Array of 1D textures
	E2DArray = 4,  // Array of 2D textures
	Cube = 5,  // Single cubemap texture
	Buffer = 6   // The dims specific to a ROTexels object
}; // enum class TexelRank

string TexelRankGetSuffix(TexelRank rank);
uint32 TexelRankGetComponentCount(TexelRank rank);


// The base types for texel formats
enum class TexelType : uint32
{
	Signed   = 0,
	Unsigned = 1,
	Float    = 2,
	UNorm    = 3,
	SNorm    = 4
};


// Describes a texel format
struct TexelFormat final
{
public:
	TexelFormat() : type{}, size{ 0 }, count{ 0 } { }
	TexelFormat(TexelType type, uint32 size, uint32 count) 
		: type{ type }, size{ size }, count{ count }
	{ }

	/* Type Check */
	bool isSame(const TexelFormat* format) const;

	/* Names */
	string getVSLName() const;
	string getGLSLName() const;
	string getVSLPrefix() const;
	string getGLSLPrefix() const;

public:
	TexelType type;
	uint32 size;
	uint32 count;
}; // struct TexelFormat


// Contains complete type information (minus array size) about an object, variable, or result
struct ShaderType final
{
public:
	ShaderType() : baseType{ BaseType::Void }, texel{} { }
	ShaderType(BaseType numericType, uint32 size, uint32 components, uint32 columns)
		: baseType{ numericType }, texel{}
	{
		numeric = { size, { components, columns } };
	}
	ShaderType(BaseType texelObjectType, TexelRank rank, const TexelFormat* format)
		: baseType{ texelObjectType }, texel{ rank, format }
	{ }
	ShaderType(BaseType structOrBufferType, const StructType* structType)
		: baseType{ structOrBufferType }, texel{}
	{
		buffer.structType = structType; // Also initializes userStruct.structType b/c union
	}

	/* Base Type Checks */
	inline bool isVoid() const     { return baseType == BaseType::Void; }
	inline bool isBoolean() const  { return baseType == BaseType::Boolean; }
	inline bool isSigned() const   { return baseType == BaseType::Signed; }
	inline bool isUnsigned() const { return baseType == BaseType::Unsigned; }
	inline bool isFloat() const    { return baseType == BaseType::Float; }
	inline bool isSampler() const  { return baseType == BaseType::Sampler; }
	inline bool isImage() const    { return baseType == BaseType::Image; }
	inline bool isROBuffer() const { return baseType == BaseType::ROBuffer; }
	inline bool isRWBuffer() const { return baseType == BaseType::RWBuffer; }
	inline bool isROTexels() const { return baseType == BaseType::ROTexels; }
	inline bool isRWTexels() const { return baseType == BaseType::RWTexels; }
	inline bool isSPInput() const  { return baseType == BaseType::SPInput; }
	inline bool isUniform() const  { return baseType == BaseType::Uniform; }
	inline bool isStruct() const   { return baseType == BaseType::Struct; }

	/* Composite Type Checks */
	inline bool isInteger() const     { return isSigned() || isUnsigned(); }
	inline bool isNumericType() const { return isInteger() || isFloat(); }
	inline bool isScalar() const      { return isNumericType() && (numeric.dims[0] == 1) && (numeric.dims[1] == 1); }
	inline bool isVector() const      { return isNumericType() && (numeric.dims[0] != 1) && (numeric.dims[1] == 1); }
	inline bool isMatrix() const      { return isNumericType() && (numeric.dims[0] != 1) && (numeric.dims[1] != 1); }
	inline bool isTexelType() const   { return isSampler() || isImage() || isROTexels() || isRWTexels() || isSPInput(); }
	inline bool isBufferType() const  { return isROBuffer() || isRWBuffer(); }
	inline bool hasStructType() const { return isUniform() || isBufferType() || isStruct(); }

	/* Casting */
	bool isSame(const ShaderType* otherType) const;
	bool hasImplicitCast(const ShaderType* targetType) const;

	/* Names */
	string getVSLName() const;
	string getGLSLName() const;

	/* Operators */
	inline bool operator == (const ShaderType& r) const { return (this == &r) || isSame(&r); }
	inline bool operator != (const ShaderType& r) const { return (this != &r) && !isSame(&r); }

public:
	BaseType baseType;
	union
	{
		struct NumericInfo
		{
			uint32 size;
			uint32 dims[2];
		} numeric;
		struct TexelInfo
		{
			TexelRank rank;
			const TexelFormat* format;
		} texel;
		struct BufferInfo
		{
			const StructType* structType;
		} buffer;
		struct StructInfo
		{
			const StructType* type;
		} userStruct;
	};
}; // struct ShaderType


// Contains a collection of types for a specific shader
class TypeList final
{
public:
	using TypeMap = std::unordered_map<string, ShaderType>;
	using StructMap = std::unordered_map<string, StructType>;
	using FormatMap = std::unordered_map<string, TexelFormat>;

	TypeList() : types_{ }, structs_{ }, error_{ } { Initialize(); }
	~TypeList() { }

	inline const string& lastError() const { return error_; }

	/* Types */
	const ShaderType* addType(const string& name, const ShaderType& type);
	const ShaderType* getType(const string& name) const;
	const StructType* addStructType(const string& name, const StructType& type);
	const StructType* getStructType(const string& name) const;
	const ShaderType* parseOrGetType(const string& name);

	/* Access */
	inline static const TypeMap& BuiltinTypes() { return BuiltinTypes_; }

private:
	static ShaderType ParseGenericType(const string& baseType);
	static const TexelFormat* GetTexelFormat(const string& format);
	static void Initialize();

private:
	TypeMap types_; // Types added by the shader, does not duplicate BuiltinTypes_
	StructMap structs_;
	mutable string error_;
	
	static bool Initialized_;
	static TypeMap BuiltinTypes_;
	static FormatMap Formats_;

	VSL_NO_COPY(TypeList)
	VSL_NO_MOVE(TypeList)
}; // class TypeList

} // namespace vsl
