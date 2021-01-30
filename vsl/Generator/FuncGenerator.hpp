/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#pragma once

#include "../Config.hpp"
#include "../ShaderInfo.hpp"


namespace vsl
{

// Used to generated GLSL function bodies from the VSL shader syntax tree
class FuncGenerator final
{
public:
	FuncGenerator(ShaderStages stage);
	~FuncGenerator();

	void emitClose();

	/* Assignment */
	void emitDeclaration(const ShaderType* type, const string& name);
	void emitVariableDefinition(const ShaderType* type, const string& name, const string& value);
	void emitAssignment(const string& left, const string& op, const string& value);
	string emitTempDefinition(const ShaderType* type, const string& value);
	void emitImageStore(const string& imStore, const string& value);

	/* Blocks */
	void emitIf(const string& cond);
	void emitElif(const string& cond);
	void emitElse();
	void emitForLoop(const string& name, int32 start, int32 end, int32 step);
	void closeBlock();

	/* Other */
	void emitControlStatement(const string& keyword);

	/* Binding */
	void emitBindingIndex(uint32 index);

	/* Source Access */
	inline const std::stringstream& source() const { return source_; }

private:
	const string name_;
	const ShaderStages stage_;
	std::stringstream source_;
	string indent_;
	uint32 uid_;
	uint32 bindingMask_;

	VSL_NO_COPY(FuncGenerator)
	VSL_NO_MOVE(FuncGenerator)
}; // class FuncGenerator

} // namespace vsl
