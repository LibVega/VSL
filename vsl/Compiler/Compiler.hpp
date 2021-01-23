/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#pragma once

#include "../Config.hpp"
#include "../Shader.hpp"


namespace vsl
{

// Performs bytecode compilation and final vbc file writes
class Compiler final
{
public:
	Compiler(const Shader* shader, const CompileOptions* options);
	~Compiler();

	void writeOutput() const;

private:
	const Shader* const shader_;
	const CompileOptions* const options_;

	VSL_NO_COPY(Compiler)
	VSL_NO_MOVE(Compiler)
}; // class Compiler

} // namespace vsl
