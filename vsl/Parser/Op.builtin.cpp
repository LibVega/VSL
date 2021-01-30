/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./Op.hpp"
#include "./Parser.hpp"


namespace vsl
{

// ====================================================================================================================
void Ops::Initialize()
{
	const string GENF{ "genType" };
	const string GENU{ "genUType" };
	const string GENI{ "genIType" };
	const string GENB{ "genBType" };
	const string DEFAULT1{ "$op$1" };
	const string DEFAULT2{ "$1 $op $2" };
	const string DEFAULT3{ "($1 ? ($2) : ($3))" };

	// Unary Ops
	Ops_["!"] = {
		{ DEFAULT1, "bool", { "bool" } },
		{ "(not($1))", GENB, { GENB } }
	};
	Ops_["~"] = {
		{ DEFAULT1, GENU, { GENU } }
	};

	// Binary Ops
	Ops_["*"] = {
		// Matrix * Matrix
		{ DEFAULT2, "float2x2", { "float2x2", "float2x2" } },
		{ DEFAULT2, "float3x3", { "float2x3", "float3x2" } },
		{ DEFAULT2, "float4x4", { "float2x4", "float4x2" } },
		{ DEFAULT2, "float2x2", { "float3x2", "float2x3" } },
		{ DEFAULT2, "float3x3", { "float3x3", "float3x3" } },
		{ DEFAULT2, "float4x4", { "float3x4", "float4x3" } },
		{ DEFAULT2, "float2x2", { "float4x2", "float2x4" } },
		{ DEFAULT2, "float3x3", { "float4x3", "float3x4" } },
		{ DEFAULT2, "float4x4", { "float4x4", "float4x4" } },
		
		// Matrix * Vector
		{ DEFAULT2, "float2", { "float2x2", "float2" } },
		{ DEFAULT2, "float3", { "float2x3", "float2" } },
		{ DEFAULT2, "float4", { "float2x4", "float2" } },
		{ DEFAULT2, "float2", { "float3x2", "float3" } },
		{ DEFAULT2, "float3", { "float3x3", "float3" } },
		{ DEFAULT2, "float4", { "float3x4", "float3" } },
		{ DEFAULT2, "float2", { "float4x2", "float4" } },
		{ DEFAULT2, "float3", { "float4x3", "float4" } },
		{ DEFAULT2, "float4", { "float4x4", "float4" } },

		// Matrix * Scalar
		{ DEFAULT2, "float2x2", { "float2x2", "float" } },
		{ DEFAULT2, "float2x3", { "float2x3", "float" } },
		{ DEFAULT2, "float2x4", { "float2x4", "float" } },
		{ DEFAULT2, "float3x2", { "float3x2", "float" } },
		{ DEFAULT2, "float3x3", { "float3x3", "float" } },
		{ DEFAULT2, "float3x4", { "float3x4", "float" } },
		{ DEFAULT2, "float4x2", { "float4x2", "float" } },
		{ DEFAULT2, "float4x3", { "float4x3", "float" } },
		{ DEFAULT2, "float4x4", { "float4x4", "float" } },

		// Vector/Scalar * Vector/Scalar
		{ DEFAULT2, GENU, { GENU, "uint" } },
		{ DEFAULT2, GENU, { GENU, GENU } },
		{ DEFAULT2, GENI, { GENI, "int" } },
		{ DEFAULT2, GENI, { GENI, GENI } },
		{ DEFAULT2, GENF, { GENF, "float" } },
		{ DEFAULT2, GENF, { GENF, GENF } }
	};
	Ops_["/"] = {
		// Matrix / Matrix
		{ DEFAULT2, "float2x2", { "float2x2", "float2x2" } },
		{ DEFAULT2, "float2x3", { "float2x3", "float2x3" } },
		{ DEFAULT2, "float2x4", { "float2x4", "float2x4" } },
		{ DEFAULT2, "float3x2", { "float3x2", "float3x2" } },
		{ DEFAULT2, "float3x3", { "float3x3", "float3x3" } },
		{ DEFAULT2, "float3x4", { "float3x4", "float3x4" } },
		{ DEFAULT2, "float4x2", { "float4x2", "float4x2" } },
		{ DEFAULT2, "float4x3", { "float4x3", "float4x3" } },
		{ DEFAULT2, "float4x4", { "float4x4", "float4x4" } },

		// Matrix / Scalar
		{ DEFAULT2, "float2x2", { "float2x2", "float" } },
		{ DEFAULT2, "float2x3", { "float2x3", "float" } },
		{ DEFAULT2, "float2x4", { "float2x4", "float" } },
		{ DEFAULT2, "float3x2", { "float3x2", "float" } },
		{ DEFAULT2, "float3x3", { "float3x3", "float" } },
		{ DEFAULT2, "float3x4", { "float3x4", "float" } },
		{ DEFAULT2, "float4x2", { "float4x2", "float" } },
		{ DEFAULT2, "float4x3", { "float4x3", "float" } },
		{ DEFAULT2, "float4x4", { "float4x4", "float" } },

		// Scalar/Vector / Scalar/Vector
		{ DEFAULT2, GENU, { GENU, "uint" } },
		{ DEFAULT2, GENU, { GENU, GENU } },
		{ DEFAULT2, GENI, { GENI, "int" } },
		{ DEFAULT2, GENI, { GENI, GENI } },
		{ DEFAULT2, GENF, { GENF, "float" } },
		{ DEFAULT2, GENF, { GENF, GENF } }
	};
	Ops_["+"] = {
		{ "$1", GENU, { GENU } }, // Unary
		{ "$1", GENI, { GENI } }, // Unary
		{ "$1", GENF, { GENF } }, // Unary

		// Matrix / Matrix
		{ DEFAULT2, "float2x2", { "float2x2", "float2x2" } },
		{ DEFAULT2, "float2x3", { "float2x3", "float2x3" } },
		{ DEFAULT2, "float2x4", { "float2x4", "float2x4" } },
		{ DEFAULT2, "float3x2", { "float3x2", "float3x2" } },
		{ DEFAULT2, "float3x3", { "float3x3", "float3x3" } },
		{ DEFAULT2, "float3x4", { "float3x4", "float3x4" } },
		{ DEFAULT2, "float4x2", { "float4x2", "float4x2" } },
		{ DEFAULT2, "float4x3", { "float4x3", "float4x3" } },
		{ DEFAULT2, "float4x4", { "float4x4", "float4x4" } },

		// Scalar/Vector + Scalar/Vector
		{ DEFAULT2, GENU, { GENU, GENU } },
		{ DEFAULT2, GENI, { GENI, GENI } },
		{ DEFAULT2, GENF, { GENF, GENF } }
	};
	Ops_["-"] = {
		{ DEFAULT1, GENI, { GENI } }, // Unary
		{ DEFAULT1, GENF, { GENF } }, // Unary

		// Matrix / Matrix
		{ DEFAULT2, "float2x2", { "float2x2", "float2x2" } },
		{ DEFAULT2, "float2x3", { "float2x3", "float2x3" } },
		{ DEFAULT2, "float2x4", { "float2x4", "float2x4" } },
		{ DEFAULT2, "float3x2", { "float3x2", "float3x2" } },
		{ DEFAULT2, "float3x3", { "float3x3", "float3x3" } },
		{ DEFAULT2, "float3x4", { "float3x4", "float3x4" } },
		{ DEFAULT2, "float4x2", { "float4x2", "float4x2" } },
		{ DEFAULT2, "float4x3", { "float4x3", "float4x3" } },
		{ DEFAULT2, "float4x4", { "float4x4", "float4x4" } },

		// Scalar/Vector + Scalar/Vector
		{ DEFAULT2, GENU, { GENU, GENU } },
		{ DEFAULT2, GENI, { GENI, GENI } },
		{ DEFAULT2, GENF, { GENF, GENF } }
	};
	Ops_["%"] = {
		{ DEFAULT2, GENU, { GENU, "uint" } },
		{ DEFAULT2, GENU, { GENU, GENU } },
		{ DEFAULT2, GENI, { GENI, "int" } },
		{ DEFAULT2, GENI, { GENI, GENI } },
		{ "(mod($1, $2))", GENF, { GENF, "float" } },
		{ "(mod($1, $2))", GENF, { GENF, GENF } }
	};
	Ops_["<<"] = {
		{ DEFAULT2, GENU, { GENU, "uint" } },
		{ DEFAULT2, GENU, { GENU, GENU } },
		{ DEFAULT2, GENI, { GENI, "int" } },
		{ DEFAULT2, GENI, { GENI, GENI } }
	};
	Ops_[">>"] = {
		{ DEFAULT2, GENU, { GENU, "uint" } },
		{ DEFAULT2, GENU, { GENU, GENU } },
		{ DEFAULT2, GENI, { GENI, "int" } },
		{ DEFAULT2, GENI, { GENI, GENI } }
	};
	Ops_["<"] = {
		{ DEFAULT2, "bool", { "uint", "uint" } },
		{ DEFAULT2, "bool", { "int", "int" } },
		{ DEFAULT2, "bool", { "float", "float" } },
		{ "lessThan($1, $2)", GENB, { GENU, GENU } },
		{ "lessThan($1, $2)", GENB, { GENI, GENI } },
		{ "lessThan($1, $2)", GENB, { GENF, GENF } }
	};
	Ops_[">"] = {
		{ DEFAULT2, "bool", { "uint", "uint" } },
		{ DEFAULT2, "bool", { "int", "int" } },
		{ DEFAULT2, "bool", { "float", "float" } },
		{ "greaterThan($1, $2)", GENB, { GENU, GENU } },
		{ "greaterThan($1, $2)", GENB, { GENI, GENI } },
		{ "greaterThan($1, $2)", GENB, { GENF, GENF } }
	};
	Ops_["<="] = {
		{ DEFAULT2, "bool", { "uint", "uint" } },
		{ DEFAULT2, "bool", { "int", "int" } },
		{ DEFAULT2, "bool", { "float", "float" } },
		{ "lessThanEqual($1, $2)", GENB, { GENU, GENU } },
		{ "lessThanEqual($1, $2)", GENB, { GENI, GENI } },
		{ "lessThanEqual($1, $2)", GENB, { GENF, GENF } }
	};
	Ops_[">="] = {
		{ DEFAULT2, "bool", { "uint", "uint" } },
		{ DEFAULT2, "bool", { "int", "int" } },
		{ DEFAULT2, "bool", { "float", "float" } },
		{ "greaterThanEqual($1, $2)", GENB, { GENU, GENU } },
		{ "greaterThanEqual($1, $2)", GENB, { GENI, GENI } },
		{ "greaterThanEqual($1, $2)", GENB, { GENF, GENF } }
	};
	Ops_["=="] = {
		{ DEFAULT2, "bool", { "uint", "uint" } },
		{ DEFAULT2, "bool", { "int", "int" } },
		{ DEFAULT2, "bool", { "float", "float" } },
		{ "equal($1, $2)", GENB, { GENU, GENU } },
		{ "equal($1, $2)", GENB, { GENI, GENI } },
		{ "equal($1, $2)", GENB, { GENF, GENF } }
	};
	Ops_["=="] = {
		{ DEFAULT2, "bool", { "uint", "uint" } },
		{ DEFAULT2, "bool", { "int", "int" } },
		{ DEFAULT2, "bool", { "float", "float" } },
		{ "notEqual($1, $2)", GENB, { GENU, GENU } },
		{ "notEqual($1, $2)", GENB, { GENI, GENI } },
		{ "notEqual($1, $2)", GENB, { GENF, GENF } }
	};
	Ops_["&"] = {
		{ DEFAULT2, GENU, { GENU, "uint" } },
		{ DEFAULT2, GENU, { GENU, GENU } },
		{ DEFAULT2, GENI, { GENI, "int" } },
		{ DEFAULT2, GENI, { GENI, GENI } }
	};
	Ops_["|"] = {
		{ DEFAULT2, GENU, { GENU, "uint" } },
		{ DEFAULT2, GENU, { GENU, GENU } },
		{ DEFAULT2, GENI, { GENI, "int" } },
		{ DEFAULT2, GENI, { GENI, GENI } }
	};
	Ops_["^"] = {
		{ DEFAULT2, GENU, { GENU, "uint" } },
		{ DEFAULT2, GENU, { GENU, GENU } },
		{ DEFAULT2, GENI, { GENI, "int" } },
		{ DEFAULT2, GENI, { GENI, GENI } }
	};
	Ops_["&&"] = {
		{ DEFAULT2, "bool", { "bool", "bool" } }
	};
	Ops_["||"] = {
		{ DEFAULT2, "bool", { "bool", "bool" } }
	};

	// Ternary
	Ops_["?:"] = {
		{ DEFAULT3, "float2x2", { "bool", "float2x2", "float2x2" } },
		{ DEFAULT3, "float2x3", { "bool", "float2x3", "float2x3" } },
		{ DEFAULT3, "float2x4", { "bool", "float2x4", "float2x4" } },
		{ DEFAULT3, "float3x2", { "bool", "float3x2", "float3x2" } },
		{ DEFAULT3, "float3x3", { "bool", "float3x3", "float3x3" } },
		{ DEFAULT3, "float3x4", { "bool", "float3x4", "float3x4" } },
		{ DEFAULT3, "float4x2", { "bool", "float4x2", "float4x2" } },
		{ DEFAULT3, "float4x3", { "bool", "float4x3", "float4x3" } },
		{ DEFAULT3, "float4x4", { "bool", "float4x4", "float4x4" } },

		{ DEFAULT3, GENU, { "bool", GENU, GENU } },
		{ DEFAULT3, GENI, { "bool", GENI, GENI } },
		{ DEFAULT3, GENF, { "bool", GENF, GENF } }
	};
}

} // namespace vsl
