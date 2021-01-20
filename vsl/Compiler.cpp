/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./Compiler.hpp"


namespace vsl
{

// ====================================================================================================================
Compiler::Compiler()
	: lastError_{ }
{
	
}

// ====================================================================================================================
Compiler::~Compiler()
{

}

// ====================================================================================================================
bool Compiler::compileFile(const string& path, const CompilerOptions& options) noexcept
{
	// Clear old compile state
	lastError_ = {};

	return true;
}

// ====================================================================================================================
bool Compiler::compileSource(const string& source, const CompilerOptions& options) noexcept
{
	return true;
}

} // namespace vsl
