/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./ScopeManager.hpp"


namespace vsl
{

// ====================================================================================================================
// ====================================================================================================================
Scope::Scope(ScopeType type)
	: type_{ type }
	, variables_{ }
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
	, scopes_{ }
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
		switch (glob.varType)
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
void ScopeManager::pushScope(Scope::ScopeType type)
{
	if (scopes_.size() == 0) {
		throw std::runtime_error("COMPILER BUG - Invalid scope pop");
	}
	scopes_.emplace_back(std::make_unique<Scope>(type));
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
bool ScopeManager::hasGlobalName(const string& name) const
{
	return hasGlobal(name);
}

// ====================================================================================================================
bool ScopeManager::inLoop() const
{
	for (int32 i = int32(scopes_.size()) - 1; i >= 0; --i) {
		if (scopes_[i]->type() == Scope::Loop) {
			return true;
		}
	}
	return false;
}

// ====================================================================================================================
void ScopeManager::PopulateBuiltins(ShaderStages stage, std::vector<Variable>& vars)
{
	const auto& bit = TypeList::BuiltinTypes();

	if (stage == ShaderStages::Vertex) {
		vars.push_back({ "$VertexIndex", VariableType::Builtin, &bit.at("int"), 1, Variable::READONLY });
		vars.push_back({ "$InstanceIndex", VariableType::Builtin, &bit.at("int"), 1, Variable::READONLY });
		vars.push_back({ "$DrawIndex", VariableType::Builtin, &bit.at("int"), 1, Variable::READONLY });
		vars.push_back({ "$VertexBase", VariableType::Builtin, &bit.at("int"), 1, Variable::READONLY });
		vars.push_back({ "$InstanceBase", VariableType::Builtin, &bit.at("int"), 1, Variable::READONLY });
		
		vars.push_back({ "$Position", VariableType::Builtin, &bit.at("float4"), 1, Variable::WRITEONLY });
		vars.push_back({ "$PointSize", VariableType::Builtin, &bit.at("float"), 1, Variable::WRITEONLY });
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
		vars.push_back({ "$FragCoord", VariableType::Builtin, &bit.at("float4"), 1, Variable::READONLY });
		vars.push_back({ "$FrontFacing", VariableType::Builtin, &bit.at("bool"), 1, Variable::READONLY });
		vars.push_back({ "$PointCoord", VariableType::Builtin, &bit.at("float2"), 1, Variable::READONLY });
		vars.push_back({ "$PrimitiveID", VariableType::Builtin, &bit.at("int"), 1, Variable::READONLY });
	}
}

} // namespace vsl
