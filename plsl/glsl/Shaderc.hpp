/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#pragma once

#include <plsl/Config.hpp>

namespace shaderc { class Compiler; }


namespace plsl
{

// Wraps an interface to a shaderc compilation task
class Shaderc final
{
public:
	Shaderc();
	~Shaderc();

private:
	std::shared_ptr<shaderc::Compiler> compiler_;

	PLSL_NO_COPY(Shaderc)
	PLSL_NO_MOVE(Shaderc)
}; // class Shaderc

} // namespace plsl
