/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#pragma once

#include <plsl/Config.hpp>
#include "../reflection/Types.hpp"


namespace plsl
{

// Used to convert names for types and builtins
class NameHelper final
{
public:
	static string GetNumericTypeName(ShaderBaseType type, uint8 size, uint8 dim0, uint8 dim1);
	static string GetBindingTypeName(const ShaderType* type, string* extra);

	static string GetImageDimsPostfix(ImageDims dims);
	static string GetImageTexelPrefix(ShaderBaseType type);
	static string GetImageTexelFormat(ShaderBaseType type, uint8 size, uint8 dim0);

	PLSL_NO_COPY(NameHelper)
	PLSL_NO_MOVE(NameHelper)
}; // class NameHelper

} // namespace plsl
