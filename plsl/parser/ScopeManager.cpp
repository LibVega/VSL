/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#include "./ScopeManager.hpp"

#include <algorithm>


namespace plsl
{

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
void ScopeManager::pushGlobalScope(ShaderStages stages)
{
	scopes_.emplace_back(std::make_unique<Scope>());
}

// ====================================================================================================================
void ScopeManager::popScope()
{
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

} // namespace plsl
