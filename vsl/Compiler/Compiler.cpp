/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./Compiler.hpp"
#include "./Reflection.hpp"

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
{

}

// ====================================================================================================================
Compiler::~Compiler()
{

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
		const auto sType = unif.type->buffer.structType;
		
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
}

} // namespace vsl
