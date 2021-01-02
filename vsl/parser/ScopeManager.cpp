/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./ScopeManager.hpp"
#include "../reflection/TypeManager.hpp"

#include <algorithm>


namespace vsl
{

// ====================================================================================================================
// ====================================================================================================================
bool Variable::canRead(ShaderStages stage) const
{
	switch (type)
	{
	case VariableType::Unknown: return false;
	case VariableType::Input: return true;
	case VariableType::Output: return false;
	case VariableType::Binding: return true;
	case VariableType::Builtin: return (extra.builtin.access == READONLY) || (extra.builtin.access == READWRITE);
	case VariableType::Constant: return true;
	case VariableType::Local: return stage == ShaderStages::Fragment; // TODO: Support more stages
	case VariableType::Parameter: return true;
	case VariableType::Private: return true;
	default: return false;
	}
}

// ====================================================================================================================
bool Variable::canWrite(ShaderStages stage) const
{
	switch (type)
	{
	case VariableType::Unknown: return false;
	case VariableType::Input: return false;
	case VariableType::Output: return true;
	case VariableType::Binding: {
		return
			(dataType->baseType == ShaderBaseType::Image) ||
			(dataType->baseType == ShaderBaseType::RWBuffer) ||
			(dataType->baseType == ShaderBaseType::RWTexels);
	} break;
	case VariableType::Builtin: return (extra.builtin.access == WRITEONLY) || (extra.builtin.access == READWRITE);
	case VariableType::Constant: return false;
	case VariableType::Local: return stage == ShaderStages::Vertex; // TODO: Support more stages
	case VariableType::Parameter: return false;
	case VariableType::Private: return true;
	default: return false;
	}
}


// ====================================================================================================================
// ====================================================================================================================
Scope::Scope()
	: variables_{ }
{

}

// ====================================================================================================================
Scope::~Scope()
{

}

// ====================================================================================================================
bool Scope::hasName(const string& name) const
{
	const auto it = std::find_if(variables_.begin(), variables_.end(), [&name](const Variable& var) {
		return var.name == name;
	});
	return it != variables_.end();
}


// ====================================================================================================================
// ====================================================================================================================
ScopeManager::ScopeManager()
	: allGlobals_{ }
	, constants_{ }
{

}

// ====================================================================================================================
ScopeManager::~ScopeManager()
{

}

// ====================================================================================================================
void ScopeManager::pushGlobalScope(ShaderStages stage)
{
	if (scopes_.size() != 0) {
		throw std::runtime_error("COMPILER BUG - Invalid scope push");
	}
	auto& scope = scopes_.emplace_back(std::make_unique<Scope>());

	// Get builtins
	PopulateBuiltins(stage, scope->variables());

	// Get specific variables for the stages
	for (const auto& glob : allGlobals_) {
		switch (glob.type)
		{
		case VariableType::Input: {
			if (stage == ShaderStages::Vertex) {
				scope->variables().push_back(glob);
			}
		} break;
		case VariableType::Output: {
			if (stage == ShaderStages::Fragment) {
				scope->variables().push_back(glob);
			}
		} break;
		case VariableType::Binding: {
			scope->variables().push_back(glob);
		} break;
		case VariableType::Constant: {
			scope->variables().push_back(glob);
		} break;
		case VariableType::Local: {
			// Always add for now, until mroe stages are supported
			scope->variables().push_back(glob);
		} break;
		}
	}
}

// ====================================================================================================================
void ScopeManager::pushScope()
{
	if (scopes_.size() == 0) {
		throw std::runtime_error("COMPILER BUG - Invalid scope pop");
	}
	scopes_.emplace_back(std::make_unique<Scope>());
}

// ====================================================================================================================
void ScopeManager::popScope()
{
	if (scopes_.size() == 0) {
		throw std::runtime_error("COMPILER BUG - Invalid scope pop");
	}
	scopes_.pop_back();
}

// ====================================================================================================================
bool ScopeManager::hasName(const string& name) const
{
	for (const auto& scope : scopes_) {
		if (scope->hasName(name)) {
			return true;
		}
	}
	return false;
}

// ====================================================================================================================
const Variable* ScopeManager::getVariable(const string& name) const
{
	for (size_t i = 0; i < scopes_.size(); ++i) {
		const auto& vars = scopes_[i]->variables();
		const auto it = std::find_if(vars.begin(), vars.end(), [&name](const Variable& var) {
			return var.name == name;
		});
		if (it != vars.end()) {
			return &(*it);
		}
	}
	return nullptr;
}

// ====================================================================================================================
void ScopeManager::addVariable(const Variable& var)
{
	if (scopes_.size() == 0) {
		throw std::runtime_error("COMPILER BUG - Invalid scope stack with variable");
	}
	scopes_.rbegin()->get()->variables().push_back(var);
}

// ====================================================================================================================
bool ScopeManager::addGlobal(const Variable& var)
{
	if (hasGlobal(var.name)) {
		return false;
	}
	allGlobals_.push_back(var);
	return true;
}

// ====================================================================================================================
bool ScopeManager::hasGlobal(const string& name) const
{
	const auto it = std::find_if(allGlobals_.begin(), allGlobals_.end(), [&name](const Variable& var) {
		return var.name == name;
	});
	return it != allGlobals_.end();
}

// ====================================================================================================================
bool ScopeManager::addConstant(const Constant& c)
{
	if (hasConstant(c.name)) {
		return false;
	}
	constants_.push_back(c);
	return true;
}

// ====================================================================================================================
bool ScopeManager::hasConstant(const string& name) const
{
	const auto it = std::find_if(constants_.begin(), constants_.end(), [&name](const Constant& c) {
		return c.name == name;
	});
	return it != constants_.end();
}

// ====================================================================================================================
const Constant* ScopeManager::getConstant(const string& name) const
{
	const auto it = std::find_if(constants_.begin(), constants_.end(), [&name](const Constant& c) {
		return c.name == name;
	});
	return (it != constants_.end()) ? &(*it) : nullptr;
}

// ====================================================================================================================
bool ScopeManager::hasGlobalName(const string& name) const
{
	return hasGlobal(name) || hasConstant(name);
}

// ====================================================================================================================
void ScopeManager::PopulateBuiltins(ShaderStages stage, std::vector<Variable>& vars)
{
	const auto& bit = TypeManager::BuiltinTypes();

	if (stage == ShaderStages::Vertex) {
		vars.push_back(Variable::Builtin("$VertexIndex", &bit.at("int"), stage, Variable::READONLY));
		vars.push_back(Variable::Builtin("$InstanceIndex", &bit.at("int"), stage, Variable::READONLY));
		vars.push_back(Variable::Builtin("$DrawIndex", &bit.at("int"), stage, Variable::READONLY));
		vars.push_back(Variable::Builtin("$VertexBase", &bit.at("int"), stage, Variable::READONLY));
		vars.push_back(Variable::Builtin("$InstanceBase", &bit.at("int"), stage, Variable::READONLY));

		vars.push_back(Variable::Builtin("$Position", &bit.at("float4"), stage, Variable::WRITEONLY));
		vars.push_back(Variable::Builtin("$PointSize", &bit.at("float"), stage, Variable::WRITEONLY));
	}
	else if (stage == ShaderStages::TessControl) {
		// TODO
	}
	else if (stage == ShaderStages::TessEval) {
		// TODO
	}
	else if (stage == ShaderStages::Geometry) {
		// TODO
	}
	else if (stage == ShaderStages::Fragment) {
		vars.push_back(Variable::Builtin("$FragCoord", &bit.at("float4"), stage, Variable::READONLY));
		vars.push_back(Variable::Builtin("$FrontFacing", &bit.at("bool"), stage, Variable::READONLY));
		vars.push_back(Variable::Builtin("$PointCoord", &bit.at("float2"), stage, Variable::READONLY));
		vars.push_back(Variable::Builtin("$PrimitiveID", &bit.at("int"), stage, Variable::READONLY));
	}
}

} // namespace vsl
