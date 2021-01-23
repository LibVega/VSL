/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#pragma once

#include "./Config.hpp"
#include "./ShaderInfo.hpp"

#define DECL_GETTER_SETTER(ftype,fname) \
	inline const ftype fname() const { return fname##_; } \
	inline ftype fname() { return fname##_; } \
	inline void fname(const ftype val) { fname##_ = val; }


namespace vsl
{

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
class CompileOptions final
{
public:
	CompileOptions() 
		: outputFile_{ "" }
		, tableSizes_{ DefaultTableSizes }
		, saveIntermediate_{ false }
		, saveBytecode_{ false }
		, disableOptimization_{ false }
		, noCompile_{ false }
	{ }
	~CompileOptions() { }

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
}; // class CompileOptions


// Describes an error that occured in the shader parse/generate/compile process
class ShaderError final
{
public:
	ShaderError(const string& msg, uint32 l = 0, uint32 c = 0)
		: message_{ msg }
		, line_{ l }
		, character_{ c }
		, badText_{ "" }
	{ }
	ShaderError()
		: ShaderError("", 0, 0)
	{ }
	~ShaderError() { }

	/* Field Access */
	DECL_GETTER_SETTER(string&, message)
	DECL_GETTER_SETTER(uint32, line)
	DECL_GETTER_SETTER(uint32, character)
	DECL_GETTER_SETTER(string&, badText)

private:
	string message_;       // The human-readable error description
	uint32 line_;          // The source line of the error (for Parse)
	uint32 character_;     // The character index in the line of the error (for Parse)
	string badText_;       // The invalid source generating the error (for Parse)
}; // class ShaderError


// Represents a shader program, which can have successive transforms (parse/generate/compile) applied to it
class Shader final
{
public:
	Shader();
	~Shader();

	/* Stages */
	inline bool isParsed() const { return progress_.parsed; }
	inline bool isGenerated() const { return progress_.generated; }
	inline bool isCompiled() const { return progress_.compiled; }
	bool parseFile(const string& path, const CompileOptions& options);
	bool parseString(const string& source, const CompileOptions& options);
	bool generate();
	bool compile();

	/* Error */
	inline const ShaderError& lastError() const { return lastError_; }
	inline bool hasError() const { return !lastError_.message().empty(); }

	/* Accessors */
	inline const ShaderInfo& info() const { return info_; }
	inline ShaderInfo& info() { return info_; }
	inline const TypeList& types() const { return types_; }
	inline TypeList& types() { return types_; }

private:
	CompileOptions options_;
	struct {
		bool parsed;
		bool generated;
		bool compiled;
	} progress_;
	ShaderError lastError_;
	ShaderInfo info_;
	TypeList types_;

public:
	static constexpr uint32 MAX_NAME_LENGTH{ 32u };      // Max length for type and variable names
	static constexpr uint32 MAX_STRUCT_SIZE{ 1024u };    // Max size in bytes for struct types
	static constexpr uint32 MAX_ARRAY_SIZE{ 64u };       // Max length for an array
	static constexpr uint32 MAX_VERTEX_ATTRIBS{ 16u };   // Maximum number of vertex attribute binding slots
	static constexpr uint32 MAX_FRAGMENT_OUTPUTS{ 8u };  // Maximum number of fragment output slots
	static constexpr uint32 MAX_BINDINGS{ 32u };         // Maximum number of resource bindings
	static constexpr uint32 MAX_SUBPASS_INPUTS{ 4u };    // Maximum number of subpass inputs
}; // class Shader

} // namespace vsl


#undef DECL_GETTER_SETTER
