/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./Shaderc.hpp"

#include <shaderc/shaderc.hpp>


namespace plsl
{

// ====================================================================================================================
Shaderc::Shaderc()
	: compiler_{ std::make_shared<shaderc::Compiler>() }
{

}

// ====================================================================================================================
Shaderc::~Shaderc()
{

}

} // namespace plsl