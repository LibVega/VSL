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

	/* Assignment */
	void emitVariableDefinition(const ShaderType* type, const string& name, const string& value);

	/* Binding */
	void emitBindingIndex(uint32 index);

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
