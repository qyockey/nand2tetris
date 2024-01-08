#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <unordered_map>

#include "Variable.h"

class SymbolTable {
public:
    SymbolTable();
    void define(const std::string& varName, const std::string& varType,
            const Variable::Kind varKind);
    bool contains(const std::string& varName);
    Variable get(const std::string& varName);
    void reset();
    void print() const;
    unsigned getNumArgs() const;
    unsigned getNumFields() const;
    unsigned getNumLocalVars() const;
    unsigned getNumStaticVars() const;

private:
    std::unordered_map<std::string, Variable> variableLookup;
    unsigned numArgs {0};
    unsigned numFieldVars {0};
    unsigned numLocalVars {0};
    unsigned numStaticVars {0};
};

#endif

