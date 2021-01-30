/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#pragma once

#include "../Config.hpp"
#include "../ShaderInfo.hpp"
#include "./FuncGenerator.hpp"


namespace vsl
{

class CompileOptions;

// Used to generate the code for a specific shader stage
class StageGenerator final
{
public:
	StageGenerator(ShaderStages stage);
	~StageGenerator();

	void generate(const FuncGenerator& func, const ShaderInfo& info);
	bool save(const CompileOptions& options);

private:
	const ShaderStages stage_;
	std::stringstream source_;

	VSL_NO_COPY(StageGenerator)
	VSL_NO_MOVE(StageGenerator)
}; // class StageGenerator

} // namespace vsl
