#ifndef PROXIMA_SYMBOL_TABLE_H
#define PROXIMA_SYMBOL_TABLE_H

#include "../lexer/Token.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace proxima {

enum class SymbolKind {
    VARIABLE,
    PARAMETER,
    FUNCTION,
    CLASS,
    INTERFACE,
    NAMESPACE,
    TYPE,
    CONSTANT
};

struct Symbol {
    std::string name;
    std::string fullName;
    std::string type;
    SymbolKind kind;
    int line;
    int column;
    std::string filename;
    std::vector<std::pair<std::string, std::string>> parameters;
    bool isGlobal;
    bool isMutable;
    
    Symbol() : kind(SymbolKind::VARIABLE), line(0), column(0), 
               isGlobal(false), isMutable(true) {}
};

class SymbolTable {
public:
    SymbolTable();
    
    void pushScope();
    void popScope();
    
    void add(const Symbol& symbol);
    Symbol get(const std::string& name);
    bool exists(const std::string& name);
    bool existsInGlobal(const std::string& name);
    
    void clear();
    
    std::vector<Symbol> getAllSymbols() const;
    std::vector<Symbol> getLocalSymbols() const;
    std::vector<Symbol> getGlobalSymbols() const;
    
    int getCurrentScope() const { return scopes.size(); }
    
private:
    std::vector<std::unordered_map<std::string, Symbol>> scopes;
    std::unordered_map<std::string, Symbol> globalSymbols;
};

} // namespace proxima

#endif // PROXIMA_SYMBOL_TABLE_H