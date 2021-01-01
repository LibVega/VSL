/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./Func.hpp"
#include "../reflection/TypeManager.hpp"

#define GETTYPE(tname) (TypeManager::GetBuiltinType(tname))


namespace plsl
{

// ====================================================================================================================
void Functions::Initialize()
{
	// See http://docs.gl/sl4/degrees for a listing of GLSL 450 functions
}

} // namespace plsl
