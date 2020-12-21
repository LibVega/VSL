/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#pragma once

#include <plsl/config.hpp>
#include "./types.hpp"

#include <vector>


namespace plsl
{

// Describes an interface variable for a shader (vertex input & fragment output)
struct InterfaceVariable final
{
public:
	InterfaceVariable() : name{}, location{}, type{} { }
	InterfaceVariable(const string& name, uint32 location, const ShaderType& type)
		: name{ name }, location{ location }, type{ type }
	{ }

public:
	string name;
	uint32 location;
	ShaderType type;
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

private:
	std::vector<InterfaceVariable> inputs_;
	std::vector<InterfaceVariable> outputs_;

	PLSL_NO_COPY(ShaderInfo)
	PLSL_NO_MOVE(ShaderInfo)
}; // class ShaderInfo

} // namespace plsl
