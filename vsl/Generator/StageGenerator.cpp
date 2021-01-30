/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./StageGenerator.hpp"
#include "./NameGeneration.hpp"
#include "../Shader.hpp"

#include <fstream>


namespace vsl
{

// ====================================================================================================================
static const string CRLF{ "\r\n" };


// ====================================================================================================================
StageGenerator::StageGenerator(const CompileOptions* options, ShaderStages stage)
	: options_{ options }
	, stage_{ stage }
	, source_{ }
	, generatedStructs_{ }
	, uid_{ 0 }
{

}

// ====================================================================================================================
StageGenerator::~StageGenerator()
{

}

// ====================================================================================================================
void StageGenerator::generate(const FuncGenerator& func, const ShaderInfo& info)
{
	// Generate header
	source_
		<< "/// This file was generated by vslc, do not edit" << CRLF
		<< "#version 450" << CRLF
		<< "#extension GL_EXT_scalar_block_layout : require" << CRLF
		<< CRLF;

	// Emit the struct types
	uint32 structCount{ 0 };
	for (const auto& bind : info.bindings()) {
		if (bool(bind.stageMask & stage_) && bind.type->hasStructType()) {
			emitStruct(bind.type->buffer.structType->userStruct.type);
			++structCount;
		}
	}
	if (bool(info.uniform().stageMask & stage_)) {
		emitStruct(info.uniform().type->buffer.structType->userStruct.type);
		++structCount;
	}
	if (structCount > 0) {
		source_ << CRLF;
	}

	// Write the stage-specific I/O
	if (bool(stage_ & ShaderStages::Vertex)) {
		for (const auto& input : info.inputs()) {
			emitVertexInput(input);
		}
		if (!info.inputs().empty()) {
			source_ << CRLF;
		}
	}
	else if (bool(stage_ & ShaderStages::Fragment)) {
		for (const auto& output : info.outputs()) {
			emitFragmentOutput(output);
		}
		if (!info.outputs().empty()) {
			source_ << CRLF;
		}
	}

	// Write the bindings
	uint32 bindCount{ 0 };
	for (const auto& bind : info.bindings()) {
		if (bool(bind.stageMask & stage_)) {
			emitBinding(bind);
			++bindCount;
		}
	}
	if (bindCount > 0) {
		source_ << CRLF;
	}

	// Write the uniform
	if (bool(info.uniform().stageMask & stage_)) {
		emitBinding(info.uniform());
		source_ << CRLF;
	}

	// Write the subpass inputs
	if (bool(stage_ & ShaderStages::Fragment) && !info.subpassInputs().empty()) {
		for (const auto& spi : info.subpassInputs()) {
			emitSubpassInput(spi);
		}
		source_ << CRLF;
	}

	// Write the binding indices
	if (bindCount > 0) {
		emitBindingIndices(info.getMaxBindingIndex());
	}

	// Write function text
	source_ << func.source().str();
}

// ====================================================================================================================
bool StageGenerator::save()
{
	if (!options_->saveIntermediate()) {
		return true;
	}

	// Open file
	const auto stage = ShaderStageToStr(stage_);
	std::ofstream file{ 
		mkstr("%s.%s.glsl", options_->outputFile().c_str(), stage.c_str()), 
		std::ofstream::trunc | std::ofstream::binary
	};
	if (!file.is_open()) {
		return false;
	}

	// Write file
	file << source_.str();
	file.flush();
	return true;
}

// ====================================================================================================================
void StageGenerator::emitStruct(const StructType* type)
{
	// Check if already emitted
	const auto dup = std::find(generatedStructs_.begin(), generatedStructs_.end(), type);
	if (dup != generatedStructs_.end()) {
		return;
	}
	generatedStructs_.push_back(type);

	// Emit the struct
	source_ << "struct " << type->name() << "_t {" << CRLF;
	for (uint32 i = 0; i < type->members().size(); ++i) {
		const auto& mem = type->members()[i];
		const auto off = type->offsets()[i];

		source_ << '\t' << mem.type->getGLSLName() << ' ' << mem.name;
		if (mem.arraySize > 1) {
			source_ << '[' << mem.arraySize << ']';
		}
		source_ << ";  // Offset: " << off << CRLF;
	}
	source_ << "};" << CRLF;
}

// ====================================================================================================================
void StageGenerator::emitVertexInput(const InterfaceVariable& var)
{
	source_ << "layout(location = " << var.location << ") in " << var.type->getGLSLName() << ' ' << var.name;
	if (var.arraySize > 1) {
		source_ << '[' << var.arraySize << ']';
	}
	source_ << ';' << CRLF;
}

// ====================================================================================================================
void StageGenerator::emitFragmentOutput(const InterfaceVariable& var)
{
	source_ 
		<< "layout(location = " << var.location << ") out " 
		<< var.type->getGLSLName() << ' ' << var.name << ';' << CRLF;
}

// ====================================================================================================================
void StageGenerator::emitBinding(const BindingVariable& bind)
{
	// Get shared binding info
	uint32 set, binding, tableSize;
	string tableName;
	getBindingInfo(bind.type, &set, &binding, &tableSize, &tableName);

	// Generate based on type
	if (bind.type->isSampler()) {
		source_
			<< "layout(set = " << set << ", binding = " << binding << ") uniform "
			<< bind.type->getGLSLName() << ' ' << tableName << '[' << tableSize << "];" << CRLF;
	}
	else if (bind.type->isImage() || bind.type->isROTexels() || bind.type->isRWTexels()) {
		auto extra = bind.type->texel.format->getGLSLName();
		source_
			<< "layout(set = " << set << ", binding = " << binding << ", " << extra << ") uniform "
			<< bind.type->getGLSLName() << ' ' << tableName << '[' << tableSize << "];" << CRLF;
	}
	else if (bind.type->isRWBuffer() || bind.type->isROBuffer()) {
		const auto access = bind.type->isROBuffer() ? "readonly" : "readwrite";
		const auto name = bind.type->buffer.structType->userStruct.type->name() + "_t";
		source_
			<< "layout(set = " << set << ", binding = " << binding << ") " << access << " buffer _BUFFER"
			<< (uid_++) << "_ {" << CRLF << '\t' << name << " _data_[];" << CRLF << 
			"} " << bind.name << '[' << tableSize << "];" << CRLF;
	}
	else if (bind.type->isUniform()) {
		const auto name = bind.type->buffer.structType->userStruct.type->name() + "_t";
		source_
			<< "layout(set = " << set << ", binding = " << binding << ") uniform _UNIFORM_ {" << CRLF
			<< '\t' << name << ' ' << bind.name << ';' << CRLF
			<< "};" << CRLF;
	}
}

// ====================================================================================================================
void StageGenerator::emitSubpassInput(const SubpassInputVariable& var)
{
	source_
		<< "layout(set = 2, binding = " << var.index << ", input_attachment_index = " << var.index << ") uniform "
		<< var.format->getGLSLPrefix() << "subpassInput " << var.name << ";" << CRLF;
}

// ====================================================================================================================
void StageGenerator::emitBindingIndices(uint32 maxIndex)
{
	const auto icount = (maxIndex + 2) / 2; // +1 to shift index->count, +1 to perform correct floor operation

	source_ << "layout(push_constant) uniform _BINDING_INDICES_ {" << CRLF;
	for (uint32 i = 0; i < icount; ++i) {
		source_ << "\tuint index" << i << ";" << CRLF;
	}
	source_ << "} _bidx_;" << CRLF << CRLF;
}

// ====================================================================================================================
void StageGenerator::getBindingInfo(const ShaderType* type, uint32* set, uint32* binding, uint32* tableSize, 
	string* tableName)
{
	// Easy
	*set = type->isUniform() ? 1 : 0;
	*tableName = NameGeneration::GetBindingTableName(type);

	// Switch on type
	switch (type->baseType)
	{
	case BaseType::Sampler: *binding = 0; *tableSize = options_->tableSizes().samplers; break;
	case BaseType::Image: *binding = 1; *tableSize = options_->tableSizes().images; break;
	case BaseType::RWBuffer:
	case BaseType::ROBuffer: *binding = 2; *tableSize = options_->tableSizes().buffers; break;
	case BaseType::ROTexels: *binding = 3; *tableSize = options_->tableSizes().roTexels; break;
	case BaseType::RWTexels: *binding = 4; *tableSize = options_->tableSizes().rwTexels; break;
	case BaseType::Uniform: *binding = 0; *tableSize = 0; break;
	}
}

} // namespace vsl
