/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#pragma once

#include "./Config.hpp"

#define DECL_GETTER_SETTER(ftype,fname) \
	inline const ftype fname() const { return fname##_; } \
	inline ftype fname() { return fname##_; } \
	inline void fname(ftype val) { fname##_ = val; }


namespace vsl
{

// The stages of the compilation process
enum class CompilerStage
{
	FileRead,
	Parse,
	Generate,
	Compile,
	FileWrite
}; // enum class CompilerStage


// Describes an error that occured in the compilation process
class CompilerError final
{
public:
	CompilerError(CompilerStage stage, const string& msg, uint32 l = 0, uint32 c = 0)
		: stage_{ stage }
		, message_{ msg }
		, line_{ l }
		, character_{ c }
		, badText_{ "" }
	{ }
	CompilerError()
		: CompilerError(CompilerStage::FileRead, "", 0, 0)
	{ }
	~CompilerError() { }

	/* Field Access */
	DECL_GETTER_SETTER(CompilerStage, stage)
	DECL_GETTER_SETTER(string&, message)
	DECL_GETTER_SETTER(uint32, line)
	DECL_GETTER_SETTER(uint32, character)
	DECL_GETTER_SETTER(string&, badText)

private:
	CompilerStage stage_;  // The compilation stage of the error
	string message_;       // The human-readable error description
	uint32 line_;          // The source line of the error (for Parse)
	uint32 character_;     // The character index in the line of the error (for Parse)
	string badText_;       // The invalid source generating the error (for Parse)
}; // class CompilerError


// Used to provide the sizes of the binding tables in generated shaders
struct BindingTableSizes final
{
public:
	uint16 samplers;
	uint16 images;
	uint16 buffers;
	uint16 roTexels;
	uint16 rwTexels;
}; // struct BindingTableSizes


// The set of options that can configure a compilation process
class CompilerOptions final
{
public:
	CompilerOptions() 
		: outputFile_{ "" }
		, tableSizes_{ DefaultTableSizes }
		, saveIntermediate_{ false }
		, saveBytecode_{ false }
		, disableOptimization_{ false }
		, noCompile_{ false }
	{ }
	~CompilerOptions() { }

	/* Field Access */
	DECL_GETTER_SETTER(string&, outputFile)
	DECL_GETTER_SETTER(BindingTableSizes&, tableSizes)
	DECL_GETTER_SETTER(bool, saveIntermediate)
	DECL_GETTER_SETTER(bool, saveBytecode)
	DECL_GETTER_SETTER(bool, disableOptimization)
	DECL_GETTER_SETTER(bool, noCompile)

public:
	// These limits require VK_EXT_descriptor_indexing for some implementations (mostly Intel integrated)
	static constexpr BindingTableSizes DefaultTableSizes{ 8192, 128, 512, 128, 128 };

private:
	string outputFile_;
	BindingTableSizes tableSizes_;
	bool saveIntermediate_;
	bool saveBytecode_;
	bool disableOptimization_;
	bool noCompile_;
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


#undef DECL_GETTER_SETTER
