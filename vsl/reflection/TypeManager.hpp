/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#pragma once

#include <vsl/Config.hpp>
#include "./Types.hpp"

#include <unordered_map>


namespace vsl
{

// Manages a collection of known types, including builtin types and user types added by shaders
class TypeManager final
{
public:
	TypeManager();
	~TypeManager();

	inline const string& lastError() const { return lastError_; } // Populated when a type function returns nullptr

	/* Type Getters */
	const ShaderType* addType(const string& name, const ShaderType& type);
	const ShaderType* getType(const string& typeName) const;
	const ShaderType* getOrAddType(const string& typeName);

	/* Type Map Access */
	inline const std::unordered_map<string, ShaderType>& addedTypes() const { return addedTypes_; }
	inline static const std::unordered_map<string, ShaderType>& BuiltinTypes() { return BuiltinTypes_; }
	static const ShaderType* GetNumericType(ShaderBaseType type, uint32 dim0, uint32 dim1);
	static const ShaderType* GetBuiltinType(const string& typeName);

	/* Parsing */
	static bool ParseTexelFormat(const string& format, ShaderType::ImageInfo::TexelInfo* info);

private:
	mutable string lastError_;
	std::unordered_map<string, ShaderType> addedTypes_; // Added types, does not duplicate BuiltinTypes_ entries

	static const std::unordered_map<string, ShaderType> BuiltinTypes_;

	VSL_NO_COPY(TypeManager)
	VSL_NO_MOVE(TypeManager)
}; // class TypeManager

} // namespace vsl
