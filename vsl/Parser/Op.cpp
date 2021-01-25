/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./Op.hpp"
#include "./Parser.hpp"

#define ERR_RETURN(msg) { LastError_ = msg; return std::make_tuple(nullptr, ""); }
#define GOOD_RETURN(type,callstr) { LastError_ = ""; return std::make_tuple(type, callstr); }


namespace vsl
{

// ====================================================================================================================
string Ops::LastError_{ };
std::unordered_map<string, std::vector<OpEntry>> Ops::Ops_{ };


// ====================================================================================================================
// ====================================================================================================================
OpType::OpType(const string& typeName)
	: type{ nullptr }
	, genType{ false }
{
	// Get the type or gentype
	if (typeName == "genType") {
		type = TypeList::GetBuiltinType("float");
		genType = true;
	}
	else if (typeName == "genIType") {
		type = TypeList::GetBuiltinType("int");
		genType = true;
	}
	else if (typeName == "genUType") {
		type = TypeList::GetBuiltinType("uint");
		genType = true;
	}
	else if (typeName == "genBType") {
		type = TypeList::GetBuiltinType("bool");
		genType = true;
	}
	else {
		type = TypeList::GetBuiltinType(typeName);
		if (!type) {
			throw std::runtime_error(
				mkstr("COMPILER BUG - Invalid type name '%s' for function type", typeName.c_str()));
		}
		genType = false;
	}
}

// ====================================================================================================================
bool OpType::match(const SPtr<Expr> expr) const
{
	const auto etype = expr->type;
	if (expr->arraySize != 1) {
		return false; // No operators take arrays as arguments
	}
	if (genType) {
		const auto casttype = TypeList::GetNumericType(type->baseType, etype->numeric.size, etype->numeric.dims[0], 1);
		return etype->hasImplicitCast(casttype);
	}
	else {
		return (type->isNumericType() || type->isBoolean()) && etype->hasImplicitCast(type);
	}
}


// ====================================================================================================================
// ====================================================================================================================
OpEntry::OpEntry(const string& genStr, const string& retTypeName, const std::vector<OpType>& args)
	: genStr{ genStr }, retType{ retTypeName }, argTypes{ args }
{ }

// ====================================================================================================================
const ShaderType* OpEntry::match(const std::vector<SPtr<Expr>>& params) const
{
	// Count check
	if (params.size() != argTypes.size()) {
		return nullptr;
	}

	// Per-arg check
	uint32 genSize = 0;
	uint32 genCount = 0;
	BaseType genType{};
	for (uint32 i = 0; i < argTypes.size(); ++i) {
		const auto& ai = argTypes[i];
		const auto& pi = params[i];
		if (!ai.match(pi)) {
			return nullptr;
		}
		if (ai.genType) {
			if (genSize == 0) {
				genSize = pi->type->numeric.size;
				genCount = pi->type->numeric.dims[0];
				genType = pi->type->baseType;
			}
			else if (genCount != pi->type->numeric.dims[0]) {
				return nullptr; // Gen types must match sizes
			}
		}
	}

	// Return the correct type
	const auto retSize =
		(!retType.genType || (genSize == 0)) ? retType.type->numeric.size :
		(retType.type->baseType != genType) ? retType.type->numeric.size : genSize;
	return retType.genType
		? TypeList::GetNumericType(retType.type->baseType, retSize, genCount, 1)
		: retType.type;
}

// ====================================================================================================================
string OpEntry::generateString(const string& op, const std::vector<SPtr<Expr>>& params) const
{
	static const string STROP{ "$op" };
	static const string STR1{ "$1" };
	static const string STR2{ "$2" };
	static const string STR3{ "$3" };

	string gen = "(" + genStr + ")";
	if (const auto posOp = gen.find(STROP); posOp != string::npos) {
		gen.replace(posOp, 3, op);
	}
	if (const auto pos1 = gen.find(STR1); pos1 != string::npos) {
		gen.replace(pos1, 2, params[0]->refString);
	}
	if (params.size() >= 2) {
		if (const auto pos2 = gen.find(STR2); pos2 != string::npos) {
			gen.replace(pos2, 2, params[1]->refString);
		}
	}
	if (params.size() >= 3) {
		if (const auto pos3 = gen.find(STR3); pos3 != string::npos) {
			gen.replace(pos3, 2, params[2]->refString);
		}
	}
	return gen;
}


// ====================================================================================================================
// ====================================================================================================================
std::tuple<const ShaderType*, string> Ops::CheckOp(const string& op, const std::vector<SPtr<Expr>>& args)
{
	if (Ops_.empty()) {
		Initialize();
	}
	const auto it = Ops_.find(op);
	if (it == Ops_.end()) {
		ERR_RETURN(mkstr("No operator '%s' found", op.c_str()));
	}
	for (const auto& entry : it->second) {
		const auto match = entry.match(args);
		if (match) {
			GOOD_RETURN(match, entry.generateString(op, args));
		}
	}
	ERR_RETURN(mkstr("No overload of operator '%s' matched the given arguments", op.c_str()));
}

} // namespace vsl
