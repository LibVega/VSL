/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#pragma once

#include "./Config.hpp"
#include "./Types.hpp"

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


// Contains information about a shader
class ShaderInfo final
{
public:
	ShaderInfo();
	~ShaderInfo();

	DECL_GETTER_SETTER(ShaderStages, stageMask)

private:
	ShaderStages stageMask_;
}; // class ShaderInfo

} // namespace vsl


#undef DECL_GETTER_SETTER
