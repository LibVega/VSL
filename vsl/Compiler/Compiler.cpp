/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./Compiler.hpp"
#include "./Reflection.hpp"
#include "../Generator/StageGenerator.hpp"

#include <shaderc/shaderc.hpp>

#include <fstream>


namespace vsl
{

// ====================================================================================================================
template<typename T>
inline static void file_write(std::ofstream& file, const T& val) {
	file.write(reinterpret_cast<const char*>(&val), sizeof(T));
}

// ====================================================================================================================
// Needed to make sure the values fit in reflection info
static_assert(Shader::MAX_NAME_LENGTH <= UINT8_MAX);
static_assert(Shader::MAX_STRUCT_SIZE <= UINT16_MAX);
static_assert(Shader::MAX_ARRAY_SIZE <= UINT8_MAX);
static_assert(Shader::MAX_VERTEX_ATTRIBS <= UINT8_MAX);
static_assert(Shader::MAX_FRAGMENT_OUTPUTS <= UINT8_MAX);
static_assert(Shader::MAX_BINDINGS <= UINT8_MAX);
static_assert(Shader::MAX_SUBPASS_INPUTS <= UINT8_MAX);
static_assert(uint32(BaseType::MAX) <= UINT8_MAX);
static_assert(uint32(TexelRank::MAX) <= UINT8_MAX);
static_assert(uint32(TexelType::MAX) <= UINT8_MAX);


// ====================================================================================================================
// ====================================================================================================================
Compiler::Compiler(const Shader* shader, const CompileOptions* options)
	: shader_{ shader }
	, options_{ options }
	, lastError_{ }
	, bytecodes_{ }
{

}

// ====================================================================================================================
Compiler::~Compiler()
{

}

// ====================================================================================================================
bool Compiler::compileStage(const StageGenerator& gen)
{
	const auto stage = gen.stage();

	// Check options
	if (options_->noCompile()) {
		return true;
	}

	// Create the compiler options
	shaderc::CompileOptions opts{};
	opts.SetOptimizationLevel(options_->disableOptimization()
		? shaderc_optimization_level_zero
		: shaderc_optimization_level_performance);
	opts.SetTargetSpirv(shaderc_spirv_version_1_5); // Vega targets Vulkan 1.2, so we can use SPIRV 1.5
	opts.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);

	// Perform compilation
	const auto skind =
		(stage == ShaderStages::Vertex) ? shaderc_vertex_shader :
		(stage == ShaderStages::TessControl) ? shaderc_tess_control_shader :
		(stage == ShaderStages::TessEval) ? shaderc_tess_evaluation_shader :
		(stage == ShaderStages::Geometry) ? shaderc_geometry_shader : shaderc_fragment_shader;
	shaderc::Compiler compiler{ };
	const auto result = compiler.CompileGlslToSpv(
		gen.source().str(),
		skind,
		"VSLC",
		"main",
		opts
	);

	// Check compile result
	if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
		lastError_ = result.GetErrorMessage();
		return false;
	}

	// Save the bytecode
	auto& bytecode = (bytecodes_[stage] = {});
	bytecode.insert(bytecode.end(), result.begin(), result.end());
	if (options_->saveBytecode() && !writeStageBytecode(stage)) {
		lastError_ = "Failed to write intermediate bytecode file";
		return false;
	}

	return true;
}

// ====================================================================================================================
void Compiler::writeOutput() const
{
	if (options_->noCompile()) {
		return;
	}

	const auto& info = shader_->info();

	// Open file and write magic number ("VBC" + uint8(1) version)
	std::ofstream file{ options_->outputFile(), std::ofstream::binary | std::ofstream::trunc };
	file << "VBC" << uint8(1);

	// Write the shader type (1 = graphics)
	file << uint8(1);

	// Write the bytecode sizes
	uint16 bytecode[5] {
		uint16(bool(info.stageMask() & ShaderStages::Vertex) ? bytecodes_.at(ShaderStages::Vertex).size() : 0),
		uint16(bool(info.stageMask() & ShaderStages::TessControl) ? bytecodes_.at(ShaderStages::TessControl).size() : 0),
		uint16(bool(info.stageMask() & ShaderStages::TessEval) ? bytecodes_.at(ShaderStages::TessEval).size() : 0),
		uint16(bool(info.stageMask() & ShaderStages::Geometry) ? bytecodes_.at(ShaderStages::Geometry).size() : 0),
		uint16(bool(info.stageMask() & ShaderStages::Fragment) ? bytecodes_.at(ShaderStages::Fragment).size() : 0)
	};
	file_write(file, bytecode);

	// Write table sizes
	file_write(file, options_->tableSizes());

	// Write vertex inputs
	file_write(file, uint32(info.inputs().size()));
	for (const auto& input : info.inputs()) {
		interface_record rec{ input };
		file_write(file, rec);
	}

	// Write fragment outputs
	file_write(file, uint32(info.outputs().size()));
	for (const auto& output : info.outputs()) {
		interface_record rec{ output };
		file_write(file, rec);
	}

	// Write bindings
	file_write(file, uint32(info.bindings().size()));
	for (const auto& binding : info.bindings()) {
		binding_record rec{ binding };
		file_write(file, rec);
	}

	// Write uniform info
	if (info.hasUniform()) {
		const auto& unif = info.uniform();
		const auto sType = unif.type->buffer.structType->userStruct.type;
		
		file_write(file, uint16(sType->size()));
		file_write(file, uint16(unif.stageMask));
		file_write(file, uint32(sType->members().size()));
		for (uint32 i = 0; i < sType->members().size(); ++i) {
			const auto& mem = sType->members()[i];
			const auto offset = sType->offsets()[i];

			file_write(file, uint8(mem.name.size()));
			file.write(mem.name.data(), mem.name.size());
			file_write(file, uint16(offset));
			struct_member_record rec{ mem };
			file_write(file, rec);
		}
	}
	else {
		file_write(file, uint16(0));
	}

	// Write subpass inputs
	file_write(file, uint32(info.subpassInputs().size()));
	for (const auto& spi : info.subpassInputs()) {
		subpass_input_record rec{ spi };
		file_write(file, spi);
	}

	// Write bytecodes
	if (bool(info.stageMask() & ShaderStages::Vertex)) {
		const auto& bc = bytecodes_.at(ShaderStages::Vertex);
		file.write(reinterpret_cast<const char*>(bc.data()), bc.size() * sizeof(uint32));
	}
	if (bool(info.stageMask() & ShaderStages::TessControl)) {
		const auto& bc = bytecodes_.at(ShaderStages::TessControl);
		file.write(reinterpret_cast<const char*>(bc.data()), bc.size() * sizeof(uint32));
	}
	if (bool(info.stageMask() & ShaderStages::TessEval)) {
		const auto& bc = bytecodes_.at(ShaderStages::TessEval);
		file.write(reinterpret_cast<const char*>(bc.data()), bc.size() * sizeof(uint32));
	}
	if (bool(info.stageMask() & ShaderStages::Geometry)) {
		const auto& bc = bytecodes_.at(ShaderStages::Geometry);
		file.write(reinterpret_cast<const char*>(bc.data()), bc.size() * sizeof(uint32));
	}
	if (bool(info.stageMask() & ShaderStages::Fragment)) {
		const auto& bc = bytecodes_.at(ShaderStages::Fragment);
		file.write(reinterpret_cast<const char*>(bc.data()), bc.size() * sizeof(uint32));
	}
}

// ====================================================================================================================
bool Compiler::writeStageBytecode(ShaderStages stage)
{
	// Open the file
	const auto stageName = ShaderStageToStr(stage);
	std::ofstream file{
		mkstr("%s.%s.spv", options_->outputFile().c_str(), stageName.c_str()),
		std::ofstream::trunc | std::ofstream::binary
	};
	if (!file.is_open()) {
		return false;
	}

	// Write file
	const auto& bytecode = bytecodes_[stage];
	file.write(reinterpret_cast<const char*>(bytecode.data()), bytecode.size() * sizeof(uint32));
	return true;
}

} // namespace vsl
