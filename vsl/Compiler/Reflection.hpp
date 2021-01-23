/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#pragma once

#include "../Config.hpp"
#include "../ShaderInfo.hpp"


namespace vsl
{

#pragma pack(push, 1)

// Used as a known layout object to write interface variable info to shader file
struct interface_record final
{
	interface_record() : location{}, baseType{}, dims{}, arraySize{}, _pad0_{} { }
	interface_record(const InterfaceVariable& var)
		: location{ uint8(var.location) }
		, baseType{ uint8(var.type->baseType) }
		, dims{ uint8(var.type->numeric.dims[0]), uint8(var.type->numeric.dims[1]) }
		, arraySize{ uint8(var.arraySize) }
		, _pad0_{ }
	{ }

	uint8 location;
	uint8 baseType;
	uint8 dims[2];
	uint8 arraySize;
	uint8 _pad0_[3];
}; // struct interface_record
static_assert(sizeof(interface_record) == 8);

// Used as a known layout object to write binding info to a shader file
struct binding_record final
{
	binding_record() : slot{}, baseType{}, stageMask{}, image{} { }
	binding_record(const BindingVariable& var)
		: slot{ uint8(var.slot) }
		, baseType{ uint8(var.type->baseType) }
		, stageMask{ uint16(var.stageMask) }
		, image{}
	{
		if (var.type->isTexelType()) {
			image.rank = uint8(var.type->texel.rank);
			image.texelType = uint8(var.type->texel.format->type);
			image.texelSize = uint8(var.type->texel.format->size);
			image.texelCount = uint8(var.type->texel.format->count);
		}
		else {
			buffer.size = uint16(var.type->buffer.structType->size());
		}
	}

	uint8 slot;
	uint8 baseType;
	uint16 stageMask;
	union {
		struct {
			uint8 rank;
			uint8 texelType;
			uint8 texelSize;
			uint8 texelCount;
		} image;
		struct {
			uint16 size;
		} buffer;
	};
}; // struct binding_record
static_assert(sizeof(binding_record) == 8);

// Used as a known layout object to write subpass input info to a shader file
struct subpass_input_record final
{
	subpass_input_record() : texelFormat{}, texelCount{}, _pad0_{} { }
	subpass_input_record(const SubpassInputVariable& var)
		: texelFormat{ uint8(var.format->type) }
		, texelCount{ uint8(var.format->count) }
		, _pad0_{ }
	{ }

	uint8 texelFormat;
	uint8 texelCount;
	uint8 _pad0_[2];
}; // struct subpass_input_record
static_assert(sizeof(subpass_input_record) == 4);

// Used as a known layout object to write struct members
struct struct_member_record final
{
	struct_member_record() : baseType{}, dims{}, arraySize{} { }
	struct_member_record(const StructType::Member& mem)
		: baseType{ uint8(mem.type->baseType) }
		, dims{ uint8(mem.type->numeric.dims[0]), uint8(mem.type->numeric.dims[1]) }
		, arraySize{ uint8(mem.arraySize) }
	{ }

	uint8 baseType;
	uint8 dims[2];
	uint8 arraySize;
}; // struct struct_memver_record
static_assert(sizeof(struct_member_record) == 4);

#pragma pack(pop)

} // namespace vsl
