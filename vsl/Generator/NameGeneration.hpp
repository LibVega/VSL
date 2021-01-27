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

// Name-related utilities for use in generation
class NameGeneration final
{
public:
	/* Binding */
	static string GetBindingTableName(const ShaderType* type);
	static string GetBindingIndexLoadString(uint32 index);

	/* Names */
	static string GetGLSLBuiltinName(const string& name);

	VSL_NO_MOVE(NameGeneration)
	VSL_NO_COPY(NameGeneration)
	VSL_NO_INIT(NameGeneration)
}; // class NameGeneration

} // namespace vsl
