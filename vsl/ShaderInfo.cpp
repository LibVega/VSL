/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./ShaderInfo.hpp"


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

} // namespace vsl
