#ifndef GLUESCRIPTCOMPILER_SCOPE_H
#define GLUESCRIPTCOMPILER_SCOPE_H

#include <string>
#include <vector>
#include <unordered_map>

struct SymbolInfo {
    std::string type;
    std::vector<std::string> args;
    bool isConst = false;
    bool isSticky = false;
    bool isNullable = false;
    std::string value;
    mutable bool stickyUsed = false;
};

class Scope {
    std::vector<std::unordered_map<std::string, SymbolInfo>> scopes;

public:
    Scope();
    void enterScope();
    void exitScope();

    bool declare(const std::string& name, const SymbolInfo& info);
    SymbolInfo* lookup(const std::string& name);
};

#endif // GLUESCRIPTCOMPILER_SCOPE_H