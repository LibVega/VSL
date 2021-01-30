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
	, spiMask_{ 0 }
{
	
}

// ====================================================================================================================
FuncGenerator::~FuncGenerator()
{

}

// ====================================================================================================================
void FuncGenerator::emitDeclaration(const ShaderType* type, const string& name)
{
	source_ << indent_ << type->getGLSLName() << " " << name << ";" << CRLF;
}

// ====================================================================================================================
void FuncGenerator::emitVariableDefinition(const ShaderType* type, const string& name, const string& value)
{
	source_ << indent_ << type->getGLSLName() << " " << name << " = " << value << ";" << CRLF;
}

// ====================================================================================================================
void FuncGenerator::emitAssignment(const string& left, const string& op, const string& value)
{
	source_ << indent_ << left << " " << op << " " << value << ";" << CRLF;
}

// ====================================================================================================================
string FuncGenerator::emitTempDefinition(const ShaderType* type, const string& value)
{
	source_ << indent_ << type->getGLSLName() << " _t" << (uid_++) << "_ = " << value << ";" << CRLF;
	return mkstr("_t%u_", uid_ - 1);
}

// ====================================================================================================================
void FuncGenerator::emitImageStore(const string& imStore, const string& value)
{
	auto repl = imStore;
	const auto repidx = repl.find("{}");
	repl.replace(repidx, 2, value);
	source_ << indent_ << repl << ";" << CRLF;
}

// ====================================================================================================================
void FuncGenerator::emitIf(const string& cond)
{
	source_ << indent_ << "if (" << cond << ") {" << CRLF;
	indent_ += '\t';
}

// ====================================================================================================================
void FuncGenerator::emitElif(const string& cond)
{
	source_ << indent_ << "else if (" << cond << ") {" << CRLF;
	indent_ += '\t';
}

// ====================================================================================================================
void FuncGenerator::emitElse()
{
	source_ << indent_ << "else {" << CRLF;
	indent_ += '\t';
}

// ====================================================================================================================
void FuncGenerator::emitForLoop(const string& name, int32 start, int32 end, int32 step)
{
	const char comp = (step > 0) ? '<' : '>';
	const char op = (step > 0) ? '+' : '-';
	source_
		<< indent_
		<< "for (int " << name << " = " << start << "; "
		<< name << ' ' << comp << ' ' << end << "; "
		<< name << ' ' << op << "= " << abs(step) << ") {\n";
	indent_ += '\t';
}

// ====================================================================================================================
void FuncGenerator::closeBlock()
{
	indent_ = indent_.substr(0, indent_.size() - 1);
	source_ << indent_ << "}" << CRLF;
}

// ====================================================================================================================
void FuncGenerator::emitControlStatement(const string& keyword)
{
	source_ << indent_ << keyword << ";" << CRLF;
}

// ====================================================================================================================
void FuncGenerator::emitBindingIndex(uint32 index)
{
	if (bindingMask_ & (1u << index)) {
		return; // Already emitted
	}
	bindingMask_ |= (1u << index);

	// Put at beginning
	const auto load = NameGeneration::GetBindingIndexLoadString(index);
	const auto saved = source_.str();
	source_ = {};
	source_ << "\tuint _bidx" << index << "_ = " << load << ";" << CRLF;
	source_ << saved;
}

} // namespace vsl
