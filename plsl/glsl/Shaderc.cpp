/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./Shaderc.hpp"

#include <shaderc/shaderc.hpp>

#include <fstream>


namespace plsl
{

// ====================================================================================================================
Shaderc::Shaderc(const CompilerOptions* options, const Generator* generator)
	: compiler_{ std::make_shared<shaderc::Compiler>() }
	, options_{ options }
	, generator_{ generator }
{

}

// ====================================================================================================================
Shaderc::~Shaderc()
{

}

// ====================================================================================================================
bool Shaderc::compileStage(ShaderStages stage) const
{
	const auto stageName = ShaderStageToStr(stage);

	// Get the stage string
	string source{};
	if (!generator_->getStageString(stage, &source)) {
		lastError_ = { CompilerStage::Compile, "Invalid stage for compilation - stage not found" };
		return false;
	}

	// For now, just save the output
	std::ofstream file{ "./plsl." + stageName };
	file << source << std::endl;

	return true;
}

} // namespace plsl
