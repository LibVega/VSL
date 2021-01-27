/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./Func.hpp"


namespace vsl
{

// ====================================================================================================================
void Functions::Initialize()
{
	const string GENF{ "genType" };
	const string GENU{ "genUType" };
	const string GENI{ "genIType" };
	const string GENB{ "genBType" };

	// See http://docs.gl/sl4/degrees for a listing of GLSL 450 functions

	// ===== TRIG FUNCTIONS =====
	Builtins_["acos"] = {
		{ "acos", GENF, { GENF } }
	};
	Builtins_["acosh"] = {
		{ "acosh", GENF, { GENF } }
	};
	Builtins_["asin"] = {
		{ "asin", GENF, { GENF } }
	};
	Builtins_["asinh"] = {
		{ "asinh", GENF, { GENF } }
	};
	Builtins_["atan"] = {
		{ "atan", GENF, { GENF } }
	};
	Builtins_["atan2"] = {
		{ "atan", GENF, { GENF, GENF } }
	};
	Builtins_["atanh"] = {
		{ "atanh", GENF, { GENF } }
	};
	Builtins_["cos"] = {
		{ "cos", GENF, { GENF } }
	};
	Builtins_["cosh"] = {
		{ "cosh", GENF, { GENF } }
	};
	Builtins_["deg2rad"] = {
		{ "radians", GENF, { GENF } }
	};
	Builtins_["rad2deg"] = {
		{ "degrees", GENF, { GENF } }
	};
	Builtins_["sin"] = {
		{ "sin", GENF, { GENF } }
	};
	Builtins_["sinh"] = {
		{ "sinh", GENF, { GENF } }
	};
	Builtins_["tan"] = {
		{ "tan", GENF, { GENF } }
	};
	Builtins_["tanh"] = {
		{ "tanh", GENF, { GENF } }
	};

	// ===== General Mathematics =====
	Builtins_["abs"] = {
		{ "abs", GENI, { GENI } },
		{ "abs", GENF, { GENF } }
	};
	Builtins_["ceil"] = {
		{ "ceil", GENF, { GENF } }
	};
	Builtins_["clamp"] = {
		{ "clamp", GENI, { GENI, "int", "int" } },
		{ "clamp", GENI, { GENI, GENI, GENI } },
		{ "clamp", GENU, { GENU, "uint", "uint" } },
		{ "clamp", GENU, { GENU, GENU, GENU } },
		{ "clamp", GENF, { GENF, "float", "float" } },
		{ "clamp", GENF, { GENF, GENF, GENF } }
	};
	// dFdy, dFdx
	Builtins_["exp"] = {
		{ "exp", GENF, { GENF } }
	};
	Builtins_["exp2"] = {
		{ "exp2", GENF, { GENF } }
	};
	Builtins_["floor"] = {
		{ "floor", GENF, { GENF } }
	};
	Builtins_["fma"] = {
		{ "fma", GENF, { GENF, GENF, GENF } }
	};
	Builtins_["fract"] = {
		{ "fract", GENF, { GENF } }
	};
	// fwidth
	Builtins_["isqrt"] = {
		{ "inverseSqrt", GENF, { GENF } }
	};
	Builtins_["isinf"] = {
		{ "isinf", GENB, { GENF } }
	};
	Builtins_["isnan"] = {
		{ "isnan", GENB, { GENF } }
	};
	Builtins_["log"] = {
		{ "log", GENF, { GENF } }
	};
	Builtins_["log2"] = {
		{ "log2", GENF, { GENF } }
	};
	Builtins_["max"] = {
		{ "max", GENI, { GENI, "int" } },
		{ "max", GENI, { GENI, GENI } },
		{ "max", GENU, { GENU, "uint" } },
		{ "max", GENU, { GENU, GENU } },
		{ "max", GENF, { GENF, "float" } },
		{ "max", GENF, { GENF, GENF } }
	};
	Builtins_["min"] = {
		{ "min", GENI, { GENI, "int" } },
		{ "min", GENI, { GENI, GENI } },
		{ "min", GENU, { GENU, "uint" } },
		{ "min", GENU, { GENU, GENU } },
		{ "min", GENF, { GENF, "float" } },
		{ "min", GENF, { GENF, GENF } }
	};
	Builtins_["mix"] = {
		{ "mix", GENB, { GENB, GENB, GENB } },
		{ "mix", GENI, { GENI, GENI, GENB } },
		{ "mix", GENU, { GENU, GENU, GENB } },
		{ "mix", GENF, { GENF, GENF, GENB } },
		{ "mix", GENF, { GENF, GENF, "float" } },
		{ "mix", GENF, { GENF, GENF, GENF } }
	};
	Builtins_["mod"] = {
		{ "mod", GENF, { GENF, "float" } },
		{ "mod", GENF, { GENF, GENF } }
	};
	Builtins_["modf"] = {
		{ "modf", GENF, { GENF, "out genType" } }
	};
	// noise
	Builtins_["pow"] = {
		{ "pow", GENF, { GENF, GENF } }
	};
	Builtins_["round"] = {
		{ "round", GENF, { GENF } }
	};
	Builtins_["roundEven"] = {
		{ "roundEven", GENF, { GENF } }
	};
	Builtins_["sign"] = {
		{ "sign", GENI, { GENI } },
		{ "sign", GENF, { GENF } }
	};
	Builtins_["smoothStep"] = {
		{ "smoothStep", GENF, { "float", "float", GENF } },
		{ "smoothStep", GENF, { GENF, GENF, GENF } }
	};
	Builtins_["sqrt"] = {
		{ "sqrt", GENF, { GENF } }
	};
	Builtins_["step"] = {
		{ "step", GENF, { "float", GENF } },
		{ "step", GENF, { GENF, GENF } }
	};
	Builtins_["trunc"] = {
		{ "trunc", GENF, { GENF } }
	};

	// ===== Floating Point Functions ===== 
	Builtins_["bitCastInt"] = {
		{ "floatBitsToInt", GENI, { GENF } }
	};
	Builtins_["bitCastUint"] = {
		{ "floatBitsToUint", GENU, { GENF } }
	};
	Builtins_["frexp"] = {
		{ "frexp", GENF, { GENF, "out genIType" } }
	};
	Builtins_["bitCastFloat"] = {
		{ "intBitsToFloat", GENF, { GENI } },
		{ "uintBitsToFloat", GENF, { GENU } },
	};
	Builtins_["ldexp"] = {
		{ "ldexp", GENF, { GENF, GENI } }
	};
	// packing and unpacking functions

	// ===== Vector Functions =====
	Builtins_["cross"] = {
		{ "cross", "float3", { "float3", "float3" } }
	};
	Builtins_["distance"] = {
		{ "distance", "float", { GENF, GENF } }
	};
	Builtins_["dot"] = {
		{ "dot", "float", { GENF, GENF } }
	};
	// equal (replaced with `vec == vec` operator)
	Builtins_["faceForward"] = {
		{ "faceForward", GENF, { GENF, GENF, GENF } }
	};
	Builtins_["length"] = {
		{ "length", "float", { GENF } }
	};
	Builtins_["normalize"] = {
		{ "normalize", GENF, { GENF } }
	};
	// notEqual (replaced with `vec != vec` operator)
	Builtins_["reflect"] = {
		{ "reflect", GENF, { GENF, GENF } }
	};
	Builtins_["refract"] = {
		{ "refract", GENF, { GENF, GENF, "float" } }
	};

	// ===== Vector Component Functions =====
	Builtins_["all"] = {
		{ "all", "bool", { GENB } }
	};
	Builtins_["any"] = {
		{ "any", "bool", { GENB } }
	};
	// greaterThan, greaterThanEqual, lessThan, lessThanEqual, not are all replaced with operators

	// ===== Integer Functions =====
	Builtins_["bitCount"] = {
		{ "bitCount", GENI, { GENI } },
		{ "bitCount", GENI, { GENU } },
	};
	// bitfieldExtract, bitfieldInsert, bitfieldReverse
	Builtins_["findLSB"] = {
		{ "findLSB", GENI, { GENI } },
		{ "findLSB", GENI, { GENU } },
	};
	Builtins_["findMSB"] = {
		{ "findMSB", GENI, { GENI } },
		{ "findMSB", GENI, { GENU } },
	};
	// uaddCarry, umulExtent, usubBorrow

	// ===== Matrix Functions =====
	Builtins_["determinant"] = {
		{ "determinant", "float", { "float2x2" } },
		{ "determinant", "float", { "float3x3" } },
		{ "determinant", "float", { "float4x4" } }
	};
	Builtins_["inverse"] = {
		{ "inverse", "float2x2", { "float2x2" } },
		{ "inverse", "float3x3", { "float3x3" } },
		{ "inverse", "float4x4", { "float4x4" } }
	};
	Builtins_["matCompMul"] = {
		{ "matrixCompMult", "float2x2", { "float2x2", "float2x2" } },
		{ "matrixCompMult", "float2x3", { "float2x3", "float2x3" } },
		{ "matrixCompMult", "float2x4", { "float2x4", "float2x4" } },
		{ "matrixCompMult", "float3x2", { "float3x2", "float3x2" } },
		{ "matrixCompMult", "float3x3", { "float3x3", "float3x3" } },
		{ "matrixCompMult", "float3x4", { "float3x4", "float3x4" } },
		{ "matrixCompMult", "float4x2", { "float4x2", "float4x2" } },
		{ "matrixCompMult", "float4x3", { "float4x3", "float4x3" } },
		{ "matrixCompMult", "float4x4", { "float4x4", "float4x4" } }
	};
	Builtins_["outerProd"] = {
		{ "outerProduct", "float2x2", { "float2", "float2" } },
		{ "outerProduct", "float2x3", { "float3", "float2" } },
		{ "outerProduct", "float2x4", { "float4", "float2" } },
		{ "outerProduct", "float3x2", { "float2", "float3" } },
		{ "outerProduct", "float3x3", { "float3", "float3" } },
		{ "outerProduct", "float3x4", { "float4", "float3" } },
		{ "outerProduct", "float4x2", { "float2", "float4" } },
		{ "outerProduct", "float4x3", { "float3", "float4" } },
		{ "outerProduct", "float4x4", { "float4", "float4" } }
	};
	Builtins_["transpose"] = {
		{ "transpose", "float2x2", { "float2x2" } },
		{ "transpose", "float2x3", { "float3x2" } },
		{ "transpose", "float2x4", { "float4x2" } },
		{ "transpose", "float3x2", { "float2x3" } },
		{ "transpose", "float3x3", { "float3x3" } },
		{ "transpose", "float3x4", { "float4x3" } },
		{ "transpose", "float4x2", { "float2x4" } },
		{ "transpose", "float4x3", { "float3x4" } },
		{ "transpose", "float4x4", { "float4x4" } }
	};

	// ===== Texture/Image Functions =====
	Builtins_["texelFetch"] = {
		{ "texelFetch", "float4", { "Sampler1D", "int", "int" } },
		{ "texelFetch", "float4", { "Sampler2D", "int2", "int" } },
		{ "texelFetch", "float4", { "Sampler3D", "int3", "int" } },
		{ "texelFetch", "float4", { "Sampler1DArray", "int2", "int" } },
		{ "texelFetch", "float4", { "Sampler2DArray", "int3", "int" } },
		{ "texelFetch", "int4",   { "ISampler1D", "int", "int" } },
		{ "texelFetch", "int4",   { "ISampler2D", "int2", "int" } },
		{ "texelFetch", "int4",   { "ISampler3D", "int3", "int" } },
		{ "texelFetch", "int4",   { "ISampler1DArray", "int2", "int" } },
		{ "texelFetch", "int4",   { "ISampler2DArray", "int3", "int" } },
		{ "texelFetch", "uint4",  { "USampler1D", "int", "int" } },
		{ "texelFetch", "uint4",  { "USampler2D", "int2", "int" } },
		{ "texelFetch", "uint4",  { "USampler3D", "int3", "int" } },
		{ "texelFetch", "uint4",  { "USampler1DArray", "int2", "int" } },
		{ "texelFetch", "uint4",  { "USampler2DArray", "int3", "int" } }
	};
	Builtins_["levelsOf"] = {
		{ "textureQueryLevels", "int", { "Sampler1D" } },
		{ "textureQueryLevels", "int", { "Sampler2D" } },
		{ "textureQueryLevels", "int", { "Sampler3D" } },
		{ "textureQueryLevels", "int", { "Sampler1DArray" } },
		{ "textureQueryLevels", "int", { "Sampler2DArray" } },
		{ "textureQueryLevels", "int", { "SamplerCube" } },
		{ "textureQueryLevels", "int", { "ISampler1D" } },
		{ "textureQueryLevels", "int", { "ISampler2D" } },
		{ "textureQueryLevels", "int", { "ISampler3D" } },
		{ "textureQueryLevels", "int", { "ISampler1DArray" } },
		{ "textureQueryLevels", "int", { "ISampler2DArray" } },
		{ "textureQueryLevels", "int", { "ISamplerCube" } },
		{ "textureQueryLevels", "int", { "USampler1D" } },
		{ "textureQueryLevels", "int", { "USampler2D" } },
		{ "textureQueryLevels", "int", { "USampler3D" } },
		{ "textureQueryLevels", "int", { "USampler1DArray" } },
		{ "textureQueryLevels", "int", { "USampler2DArray" } },
		{ "textureQueryLevels", "int", { "USamplerCube" } },
	};
	Builtins_["sizeOf"] = {
		{ "textureSize", "int",  { "Sampler1D", "int" } },
		{ "textureSize", "int2", { "Sampler2D", "int" } },
		{ "textureSize", "int3", { "Sampler3D", "int" } },
		{ "textureSize", "int2", { "Sampler1DArray", "int" } },
		{ "textureSize", "int3", { "Sampler2DArray", "int" } },
		{ "textureSize", "int2", { "SamplerCube", "int" } },
		{ "textureSize", "int",  { "ISampler1D", "int" } },
		{ "textureSize", "int2", { "ISampler2D", "int" } },
		{ "textureSize", "int3", { "ISampler3D", "int" } },
		{ "textureSize", "int2", { "ISampler1DArray", "int" } },
		{ "textureSize", "int3", { "ISampler2DArray", "int" } },
		{ "textureSize", "int2", { "ISamplerCube", "int" } },
		{ "textureSize", "int",  { "USampler1D", "int" } },
		{ "textureSize", "int2", { "USampler2D", "int" } },
		{ "textureSize", "int3", { "USampler3D", "int" } },
		{ "textureSize", "int2", { "USampler1DArray", "int" } },
		{ "textureSize", "int3", { "USampler2DArray", "int" } },
		{ "textureSize", "int2", { "USamplerCube", "int" } },
		//{ "imageSize", "int",  { "Image1D" } },
		//{ "imageSize", "int2", { "Image2D" } },
		//{ "imageSize", "int3", { "Image3D" } },
		//{ "imageSize", "int2", { "Image1DArray" } },
		//{ "imageSize", "int3", { "Image2DArray" } }
	};
}

} // namespace vsl
