/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./NameGeneration.hpp"

#include <algorithm>
#include <unordered_map>


namespace vsl
{

// ====================================================================================================================
string NameGeneration::GetBindingTableName(const ShaderType* type)
{
	string basename{ "INVALID" };
	switch (type->baseType)
	{
	case BaseType::Sampler: {
		const auto rtxt = TexelRankGetSuffix(type->texel.rank);
		basename = "sampler" + rtxt;
	} break;
	case BaseType::Image: {
		const auto rtxt = TexelRankGetSuffix(type->texel.rank);
		const auto ftxt = type->texel.format->getVSLName();
		basename = "image" + rtxt + "_" + ftxt;
	} break;
	case BaseType::ROTexels: {
		const auto fix = type->texel.format->getGLSLPrefix();
		basename = fix + "rotexels";
	} break;
	case BaseType::RWTexels: {
		const auto ftxt = type->texel.format->getVSLName();
		basename = "rwtexels_" + ftxt;
	} break;
	}

	// Create final name
	std::transform(basename.begin(), basename.end(), basename.begin(), ::toupper);
	return "_" + basename + "_TABLE_";
}

// ====================================================================================================================
string NameGeneration::GetBindingIndexLoadString(uint32 index)
{
	return ((index & 1) == 0)
		? mkstr("(_bidx_.index%u & 0x0000FFFF)", index / 2)
		: mkstr("(_bidx_.index%u >> 16)", index / 2);
}

// ====================================================================================================================
string NameGeneration::GetGLSLBuiltinName(const string& name)
{
	static const std::unordered_map<string, string> NAMES { 
		{ "$VertexIndex", "gl_VertexIndex" },
		{ "$InstanceIndex", "gl_InstanceIndex" },
		{ "$DrawIndex", "gl_DrawIndex" },
		{ "$VertexBase", "gl_BaseVertex" },
		{ "$InstanceBase", "gl_BaseInstance" },

		{ "$Position", "gl_Position" },
		{ "$PointSize", "gl_PointSize" },

		{ "$FragCoord", "gl_FragCoord" },
		{ "$FrontFacing", "gl_FrontFacing" },
		{ "$PointCoord", "gl_PointCoord" },
		{ "$PrimitiveID", "gl_PrimitiveID" }
	};

	const auto it = NAMES.find(name);
	if (it != NAMES.end()) {
		return it->second;
	}
	return "INVALID";
}

} // namespace vsl
