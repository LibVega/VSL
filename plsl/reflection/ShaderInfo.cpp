/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./ShaderInfo.hpp"


namespace plsl
{

// ====================================================================================================================
ShaderInfo::ShaderInfo()
	: inputs_{ }
	, outputs_{ }
{

}

// ====================================================================================================================
ShaderInfo::~ShaderInfo()
{

}

// ====================================================================================================================
const InterfaceVariable* ShaderInfo::getInput(const string& name) const
{
	const auto it = std::find_if(inputs_.begin(), inputs_.end(), [&name](const InterfaceVariable& var) {
		return var.name == name;
	});
	return (it != inputs_.end()) ? &(*it) : nullptr;
}

// ====================================================================================================================
const InterfaceVariable* ShaderInfo::getInput(uint32 location) const
{
	const auto it = std::find_if(inputs_.begin(), inputs_.end(), [location](const InterfaceVariable& var) {
		return (var.location == location) || (location < (var.location + var.bindingCount()));
	});
	return (it != inputs_.end()) ? &(*it) : nullptr;
}

// ====================================================================================================================
const InterfaceVariable* ShaderInfo::getOutput(const string& name) const
{
	const auto it = std::find_if(outputs_.begin(), outputs_.end(), [&name](const InterfaceVariable& var) {
		return var.name == name;
	});
	return (it != outputs_.end()) ? &(*it) : nullptr;
}

// ====================================================================================================================
const InterfaceVariable* ShaderInfo::getOutput(uint32 location) const
{
	const auto it = std::find_if(outputs_.begin(), outputs_.end(), [location](const InterfaceVariable& var) {
		return var.location == location;
	});
	return (it != outputs_.end()) ? &(*it) : nullptr;
}

} // namespace plsl
