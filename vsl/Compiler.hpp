/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#pragma once

#include "./Config.hpp"


namespace vsl
{

// Describes an error that occured in the compilation process
class CompilerError final
{
public:
	inline string message() const { return ""; }
}; // class CompilerError


// The set of options that can configure a compilation process
class CompilerOptions final
{

}; // class CompilerOptions


// The core compiler type, which performs the full configurable compilation process
class VSL_API Compiler final
{
public:
	Compiler();
	~Compiler();

	bool compileFile(const string& path, const CompilerOptions& options) noexcept;
	bool compileSource(const string& source, const CompilerOptions& options) noexcept;

	inline const CompilerError& lastError() const { return lastError_; }
	inline bool hasError() const { return !lastError_.message().empty(); }

private:
	CompilerError lastError_;

	VSL_NO_COPY(Compiler)
	VSL_NO_MOVE(Compiler)
}; // class Compiler

} // namespace vsl
