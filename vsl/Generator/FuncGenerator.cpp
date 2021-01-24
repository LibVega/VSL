/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./FuncGenerator.hpp"
#include "./NameGeneration.hpp"


namespace vsl
{

// ====================================================================================================================
static const string CRLF{ "\r\n" };


// ====================================================================================================================
FuncGenerator::FuncGenerator(ShaderStages stage)
	: name_{ "main" }
	, stage_{ stage }
	, source_{ }
	, indent_{ "\t" }
	, uid_{ 0 }
	, bindingMask_{ 0 }
{
	source_ << "void main()" << CRLF << "{" << CRLF;
}

// ====================================================================================================================
FuncGenerator::~FuncGenerator()
{

}

// ====================================================================================================================
void FuncGenerator::emitVariableDefinition(const ShaderType* type, const string& name, const string& value)
{
	source_ << indent_ << type->getGLSLName() << " " << name << " = " << value << ";" << CRLF;
}

// ====================================================================================================================
string FuncGenerator::emitTempDefinition(const ShaderType* type, const string& value)
{
	source_ << indent_ << type->getGLSLName() << " _t" << (uid_++) << "_ = " << value << ";" << CRLF;
	return mkstr("_t%u_", uid_ - 1);
}

// ====================================================================================================================
void FuncGenerator::emitBindingIndex(uint32 index)
{
	if (bindingMask_ & (1u << index)) {
		return; // Already emitted
	}
	bindingMask_ |= (1u << index);

	const auto load = NameGeneration::GetBindingIndexLoadString(index);
	source_ << indent_ << "uint _bidx" << index << "_ = " << load << ";" << CRLF;
}

} // namespace vsl
