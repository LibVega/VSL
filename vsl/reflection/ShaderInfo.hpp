/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#pragma once

#include <vsl/Config.hpp>
#include "./Types.hpp"

#include <vector>

#define VSL_MAX_INPUT_INDEX      (31u)  // Max binding index for vertex inputs
#define VSL_MAX_OUTPUT_INDEX     (7u)   // Max binding index for fragment outputs
#define VSL_MAX_INPUT_ARRAY_SIZE (8u)   // Max array size for vertex inputs
#define VSL_MAX_BINDING_INDEX    (31u)  // Max binding index for bindings
#define VSL_MAX_SUBPASS_INPUTS   (4u)   // Max number of subpass inputs


namespace vsl
{

// Describes an interface variable for a shader (vertex input & fragment output)
struct InterfaceVariable final
{
public:
	InterfaceVariable() : name{}, location{}, type{}, arraySize{} { }
	InterfaceVariable(const string& name, uint32 location, const ShaderType& type, uint8 arrSize)
		: name{ name }, location{ location }, type{ type }, arraySize{ arrSize }
	{ }

	inline uint32 bindingCount() const { return type.getBindingCount() * arraySize; }

public:
	string name;
	uint32 location;
	ShaderType type;
	uint8 arraySize;
}; // struct InterfaceVariable


// Describes a binding variable
struct BindingVariable final
{
public:
	BindingVariable() : name{}, type{}, slot{} { }
	BindingVariable(const string& name, const ShaderType& type, uint8 slot)
		: name{ name }, type{ type }, slot{ slot }
	{ }

public:
	string name;
	ShaderType type;
	uint8 slot;
}; // struct BindingVariable


// Describes an input attachment
struct SubpassInput final
{
public:
	SubpassInput() : name{}, type{}, index{} { }
	SubpassInput(const string& name, ShaderBaseType type, uint8 index)
		: name{ name }, type{ type }, index{ index }
	{ }

public:
	string name;
	ShaderBaseType type;
	uint8 index;
}; // struct SubpassInput


// The different shader stages as a bitmask
enum class ShaderStages : uint16
{
	None        = 0,
	Vertex      = (1 << 0),
	TessControl = (1 << 1),
	TessEval    = (1 << 2),
	Geometry    = (1 << 3),
	Fragment    = (1 << 4),
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


// Contains information about the public-facing interface of a shader program
class ShaderInfo final
{
public:
	ShaderInfo();
	~ShaderInfo();

	// Top-Level Info
	inline ShaderStages stages() const { return stages_; }
	inline void addStage(ShaderStages stage) { stages_ |= stage; }

	// Accessors
	inline const std::vector<InterfaceVariable>& inputs() const { return inputs_; }
	inline std::vector<InterfaceVariable>& inputs() { return inputs_; }
	inline const std::vector<InterfaceVariable>& outputs() const { return outputs_; }
	inline std::vector<InterfaceVariable>& outputs() { return outputs_; }
	inline const std::vector<SubpassInput>& subpassInputs() const { return subpassInputs_; }
	inline std::vector<SubpassInput>& subpassInputs() { return subpassInputs_; }
	inline const std::vector<BindingVariable>& bindings() const { return bindings_; }
	inline std::vector<BindingVariable>& bindings() { return bindings_; }

	// Interface Variables
	const InterfaceVariable* getInput(const string& name) const;
	const InterfaceVariable* getInput(uint32 location) const; // Is aware of inputs larger than one binding
	const InterfaceVariable* getOutput(const string& name) const;
	const InterfaceVariable* getOutput(uint32 location) const;

	// Bindings
	const SubpassInput* getSubpassInput(const string& name) const;
	const SubpassInput* getSubpassInput(uint8 index) const;
	const BindingVariable* getBinding(const string& name) const;
	const BindingVariable* getBinding(uint8 slotIndex) const;
	uint32 getMaxBindingIndex() const;

private:
	ShaderStages stages_;
	std::vector<InterfaceVariable> inputs_;
	std::vector<InterfaceVariable> outputs_;
	std::vector<SubpassInput> subpassInputs_;
	std::vector<BindingVariable> bindings_;

	VSL_NO_COPY(ShaderInfo)
	VSL_NO_MOVE(ShaderInfo)
}; // class ShaderInfo

} // namespace vsl
