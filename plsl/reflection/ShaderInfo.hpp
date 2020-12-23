/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#pragma once

#include <plsl/Config.hpp>
#include "./Types.hpp"

#include <vector>

#define PLSL_MAX_INPUT_INDEX      (31u)  // Max binding index for vertex inputs
#define PLSL_MAX_OUTPUT_INDEX     (7u)   // Max binding index for fragment outputs
#define PLSL_MAX_INPUT_ARRAY_SIZE (8u)   // Max array size for vertex inputs


namespace plsl
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


// Contains information about the public-facing interface of a shader program
class ShaderInfo final
{
public:
	ShaderInfo();
	~ShaderInfo();

	// Accessors
	inline const std::vector<InterfaceVariable>& inputs() const { return inputs_; }
	inline std::vector<InterfaceVariable>& inputs() { return inputs_; }
	inline const std::vector<InterfaceVariable>& outputs() const { return outputs_; }
	inline std::vector<InterfaceVariable>& outputs() { return outputs_; }

	// Interface Variables
	const InterfaceVariable* getInput(const string& name) const;
	const InterfaceVariable* getInput(uint32 location) const; // Is aware of inputs larger than one binding
	const InterfaceVariable* getOutput(const string& name) const;
	const InterfaceVariable* getOutput(uint32 location) const;

private:
	std::vector<InterfaceVariable> inputs_;
	std::vector<InterfaceVariable> outputs_;

	PLSL_NO_COPY(ShaderInfo)
	PLSL_NO_MOVE(ShaderInfo)
}; // class ShaderInfo

} // namespace plsl