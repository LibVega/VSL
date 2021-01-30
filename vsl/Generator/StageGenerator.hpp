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
	StageGenerator(const CompileOptions* options, ShaderStages stage);
	~StageGenerator();

	inline ShaderStages stage() const { return stage_; }
	inline const std::stringstream& source() const { return source_; }

	void generate(const FuncGenerator& func, const ShaderInfo& info);
	bool save();

private:
	void emitStruct(const StructType* type);
	void emitVertexInput(const InterfaceVariable& var);
	void emitFragmentOutput(const InterfaceVariable& var);
	void emitBinding(const BindingVariable& bind);
	void emitSubpassInput(const SubpassInputVariable& var);
	void emitBindingIndices(uint32 maxIndex);
	void emitLocal(const LocalVariable& var);

	void getBindingInfo(const ShaderType* type, uint32* set, uint32* binding, uint32* tableSize, string* tableName);

private:
	const CompileOptions* const options_;
	const ShaderStages stage_;
	std::stringstream source_;
	std::vector<const StructType*> generatedStructs_;
	uint32 uid_;
	struct {
		uint32 in;
		uint32 out;
	} localIdx_;

	VSL_NO_COPY(StageGenerator)
	VSL_NO_MOVE(StageGenerator)
}; // class StageGenerator

} // namespace vsl
