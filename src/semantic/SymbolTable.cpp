#include "SymbolTable.h"

namespace proxima {

SymbolTable::SymbolTable() {
    pushScope(); // Global scope
}

void SymbolTable::pushScope() {
    scopes.emplace_back();
}

void SymbolTable::popScope() {
    if (scopes.size() > 1) {
        scopes.pop_back();
    }
}

void SymbolTable::add(const Symbol& symbol) {
    if (symbol.isGlobal) {
        globalSymbols[symbol.fullName] = symbol;
    }
    scopes.back()[symbol.fullName] = symbol;
}

Symbol SymbolTable::get(const std::string& name) {
    // Search from innermost to outermost scope
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        auto search = it->find(name);
        if (search != it->end()) {
            return search->second;
        }
    }
    
    // Search global scope
    auto search = globalSymbols.find(name);
    if (search != globalSymbols.end()) {
        return search->second;
    }
    
    return Symbol(); // Return empty symbol if not found
}

bool SymbolTable::exists(const std::string& name) {
    for (const auto& scope : scopes) {
        if (scope.find(name) != scope.end()) {
            return true;
        }
    }
    return globalSymbols.find(name) != globalSymbols.end();
}

bool SymbolTable::existsInGlobal(const std::string& name) {
    return globalSymbols.find(name) != globalSymbols.end();
}

void SymbolTable::clear() {
    scopes.clear();
    globalSymbols.clear();
    pushScope();
}

std::vector<Symbol> SymbolTable::getAllSymbols() const {
    std::vector<Symbol> allSymbols;
    
    for (const auto& scope : scopes) {
        for (const auto& pair : scope) {
            allSymbols.push_back(pair.second);
        }
    }
    
    for (const auto& pair : globalSymbols) {
        allSymbols.push_back(pair.second);
    }
    
    return allSymbols;
}

std::vector<Symbol> SymbolTable::getLocalSymbols() const {
    std::vector<Symbol> localSymbols;
    
    if (!scopes.empty()) {
        for (const auto& pair : scopes.back()) {
            localSymbols.push_back(pair.second);
        }
    }
    
    return localSymbols;
}

std::vector<Symbol> SymbolTable::getGlobalSymbols() const {
    std::vector<Symbol> globalSyms;
    
    for (const auto& pair : globalSymbols) {
        globalSyms.push_back(pair.second);
    }
    
    return globalSyms;
}

} // namespace proxima