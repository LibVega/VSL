/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./ShaderInfo.hpp"


namespace plsl
{

// ====================================================================================================================
ShaderStages StrToShaderStage(const string& str)
{
	if (str == "vert") {
		return ShaderStages::Vertex;
	}
	if (str == "tesc") {
		return ShaderStages::TessControl;
	}
	if (str == "tese") {
		return ShaderStages::TessEval;
	}
	if (str == "geom") {
		return ShaderStages::Geometry;
	}
	if (str == "frag") {
		return ShaderStages::Fragment;
	}
	return ShaderStages::None;
}

// ====================================================================================================================
string ShaderStageToStr(ShaderStages stage)
{
	switch (stage)
	{
	case ShaderStages::Vertex: return "vert";
	case ShaderStages::TessControl: return "tesc";
	case ShaderStages::TessEval: return "tese";
	case ShaderStages::Geometry: return "geom";
	case ShaderStages::Fragment: return "frag";
	default: return "";
	}
}

// ====================================================================================================================
// ====================================================================================================================
ShaderInfo::ShaderInfo()
	: stages_{ ShaderStages::None }
	, inputs_{ }
	, outputs_{ }
	, bindings_{ }
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

// ====================================================================================================================
const BindingVariable* ShaderInfo::getBinding(const string& name) const
{
	const auto it = std::find_if(bindings_.begin(), bindings_.end(), [&name](const BindingVariable& bind) {
		return bind.name == name;
	});
	return (it != bindings_.end()) ? &(*it) : nullptr;
}

// ====================================================================================================================
const BindingVariable* ShaderInfo::getBinding(BindingGroup group, uint8 slotIndex) const
{
	const auto it = std::find_if(bindings_.begin(), bindings_.end(), [group, slotIndex](const BindingVariable& bind) {
		return bind.group == group && bind.slotIndex == slotIndex;
	});
	return (it != bindings_.end()) ? &(*it) : nullptr;
}

} // namespace plsl
