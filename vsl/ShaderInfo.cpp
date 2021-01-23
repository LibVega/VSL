/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./ShaderInfo.hpp"

#include <algorithm>


namespace vsl
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
	: stageMask_{ ShaderStages::None }
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
const SubpassInputVariable* ShaderInfo::getSubpassInput(const string& name) const
{
	const auto it = std::find_if(subpassInputs_.begin(), subpassInputs_.end(), [&name](const SubpassInputVariable& si) {
		return si.name == name;
	});
	return (it != subpassInputs_.end()) ? &(*it) : nullptr;
}

// ====================================================================================================================
const SubpassInputVariable* ShaderInfo::getSubpassInput(uint32 index) const
{
	const auto it = std::find_if(subpassInputs_.begin(), subpassInputs_.end(), [index](const SubpassInputVariable& si) {
		return si.index == index;
	});
	return (it != subpassInputs_.end()) ? &(*it) : nullptr;
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
const BindingVariable* ShaderInfo::getBinding(uint32 slotIndex) const
{
	const auto it = std::find_if(bindings_.begin(), bindings_.end(), [slotIndex](const BindingVariable& bind) {
		return bind.slot == slotIndex;
	});
	return (it != bindings_.end()) ? &(*it) : nullptr;
}

// ====================================================================================================================
BindingVariable* ShaderInfo::getBinding(const string& name)
{
	const auto it = std::find_if(bindings_.begin(), bindings_.end(), [&name](const BindingVariable& bind) {
		return bind.name == name;
	});
	return (it != bindings_.end()) ? &(*it) : nullptr;
}

// ====================================================================================================================
BindingVariable* ShaderInfo::getBinding(uint32 slotIndex)
{
	const auto it = std::find_if(bindings_.begin(), bindings_.end(), [slotIndex](const BindingVariable& bind) {
		return bind.slot == slotIndex;
	});
	return (it != bindings_.end()) ? &(*it) : nullptr;
}

// ====================================================================================================================
uint32 ShaderInfo::getMaxBindingIndex() const
{
	const auto it =
		std::max_element(bindings_.begin(), bindings_.end(), [](const BindingVariable& left, const BindingVariable& right) {
			return left.slot < right.slot;
		});
	return (it != bindings_.end()) ? it->slot : 0;
}

} // namespace vsl
