/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#pragma once

#include "./Config.hpp"
#include "./Types.hpp"

#include <vector>

#define DECL_GETTER_SETTER(ftype,fname) \
	inline const ftype fname() const { return fname##_; } \
	inline ftype fname() { return fname##_; } \
	inline void fname(const ftype val) { fname##_ = val; }


namespace vsl
{

// The different shader stages as a bitmask
enum class ShaderStages : uint32
{
	None = 0,
	Vertex = (1 << 0),
	TessControl = (1 << 1),
	TessEval = (1 << 2),
	Geometry = (1 << 3),
	Fragment = (1 << 4),
	AllGraphics = Vertex | TessControl | TessEval | Geometry | Fragment
}; // enum class ShaderStages
inline ShaderStages operator | (ShaderStages l, ShaderStages r) { return ShaderStages(uint16(l) | uint16(r)); }
inline ShaderStages operator & (ShaderStages l, ShaderStages r) { return ShaderStages(uint16(l) & uint16(r)); }
inline ShaderStages& operator |= (ShaderStages& l, ShaderStages r) {
	l = l | r;
	return l;
}
inline ShaderStages& operator &= (ShaderStages& l, ShaderStages r) {
	l = l & r;
	return l;
}
ShaderStages StrToShaderStage(const string& str);
string ShaderStageToStr(ShaderStages stage);


// Describes an interface variable for a shader (vertex input & fragment output)
struct InterfaceVariable final
{
public:
	InterfaceVariable() : name{}, location{}, type{}, arraySize{} { }
	InterfaceVariable(const string& name, uint32 location, const ShaderType* type, uint32 arrSize)
		: name{ name }, location{ location }, type{ type }, arraySize{ arrSize }
	{ }

	inline uint32 bindingCount() const { return type->getBindingCount() * arraySize; }

public:
	string name;
	uint32 location;
	const ShaderType* type;
	uint32 arraySize;
}; // struct InterfaceVariable


// Describes a binding variable, uniform, or subpass input
struct BindingVariable final
{
public:
	BindingVariable() : name{}, type{}, slot{}, stageMask{} { }
	BindingVariable(const string& name, const ShaderType* type, uint32 slot)
		: name{ name }, type{ type }, slot{ slot }, stageMask{}
	{ }

public:
	string name;
	const ShaderType* type;
	uint32 slot;
	ShaderStages stageMask; // Shader stages that use the binding
}; // struct BindingVariable


// Describes an input attachment
struct SubpassInputVariable final
{
public:
	SubpassInputVariable() : name{}, index{ 0 }, format{ nullptr } { }
	SubpassInputVariable(const string& name, uint32 index, const TexelFormat* format)
		: name{ name }, index{ index }, format{ format }
	{ }

public:
	string name;
	uint32 index;
	const TexelFormat* format;
}; // struct SubpassInputVariable


// Contains information about a shader
class ShaderInfo final
{
public:
	ShaderInfo();
	~ShaderInfo();

	DECL_GETTER_SETTER(ShaderStages, stageMask)
	DECL_GETTER_SETTER(std::vector<InterfaceVariable>, inputs)
	DECL_GETTER_SETTER(std::vector<InterfaceVariable>, outputs)
	DECL_GETTER_SETTER(std::vector<BindingVariable>, bindings)
	DECL_GETTER_SETTER(BindingVariable, uniform)
	DECL_GETTER_SETTER(std::vector<SubpassInputVariable>, subpassInputs)

	/* Interface Variables */
	const InterfaceVariable* getInput(const string& name) const;
	const InterfaceVariable* getInput(uint32 location) const; // Is aware of inputs larger than one binding
	const InterfaceVariable* getOutput(const string& name) const;
	const InterfaceVariable* getOutput(uint32 location) const;

	/* Bindings */
	const SubpassInputVariable* getSubpassInput(const string& name) const;
	const SubpassInputVariable* getSubpassInput(uint32 index) const;
	const BindingVariable* getBinding(const string& name) const;
	const BindingVariable* getBinding(uint32 slotIndex) const;
	BindingVariable* getBinding(const string& name);
	BindingVariable* getBinding(uint32 slotIndex);
	inline bool hasUniform() const { return !uniform_.name.empty(); }
	uint32 getMaxBindingIndex() const;

private:
	ShaderStages stageMask_;
	std::vector<InterfaceVariable> inputs_;
	std::vector<InterfaceVariable> outputs_;
	std::vector<BindingVariable> bindings_;
	BindingVariable uniform_;
	std::vector<SubpassInputVariable> subpassInputs_;

	VSL_NO_COPY(ShaderInfo)
	VSL_NO_MOVE(ShaderInfo)
}; // class ShaderInfo

} // namespace vsl


#undef DECL_GETTER_SETTER
