/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#pragma once

#include "./config.hpp"


namespace plsl
{

class PLSL_API Compiler final
{
public:
	Compiler();
	~Compiler();

private:

	PLSL_NO_COPY(Compiler)
	PLSL_NO_MOVE(Compiler)
}; // class Compiler

} // namespace plsl
