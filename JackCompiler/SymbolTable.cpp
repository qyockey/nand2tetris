#include <iostream>

#include "SymbolTable.h"
#include "UnexpectedTokenException.h"

SymbolTable::SymbolTable() = default;

void SymbolTable::define(const std::string& varName, const std::string& varType,
        const Variable::Kind varKind) {
    unsigned varNumber;
    switch (varKind) {
        case Variable::Kind::ARG:
            varNumber = numArgs;
            numArgs++;
            break;
        case Variable::Kind::FIELD:
            varNumber = numFieldVars;
            numFieldVars++;
            break;
        case Variable::Kind::STATIC:
            varNumber = numStaticVars;
            numStaticVars++;
            break;
        case Variable::Kind::VAR:
            varNumber = numLocalVars;
            numLocalVars++;
            break;
    }
    try {
        variableLookup.at(varName);
        throw UnexpectedTokenException("Duplicate variable declaration");
    } catch (const std::out_of_range&) {
        variableLookup.try_emplace(varName, varName, varType, varKind,
                varNumber);
    }
}

bool SymbolTable::contains(const std::string& varName) {
    try {
        variableLookup.at(varName);
        return true;
    } catch (const std::out_of_range&) {
        return false;
    }
}

Variable SymbolTable::get(const std::string& varName) {
    return variableLookup.at(varName);
}

void SymbolTable::reset() {
    variableLookup.clear();
    numArgs = 0;
    numFieldVars = 0;
    numLocalVars = 0;
    numStaticVars = 0;
}

void SymbolTable::print() const {
    for (const auto& [name, variable] : variableLookup) {
        std::cout << name << ' ' << variable.getType() << ' '
                << Variable::kindToStr(variable.getKind()) << ' '
                << variable.getNumber() << '\n';
    }
    std::cout << '\n';
}

unsigned SymbolTable::getNumArgs() const {
    return numArgs;
}

unsigned SymbolTable::getNumFields() const {
    return numFieldVars;
}

unsigned SymbolTable::getNumLocalVars() const {
    return numLocalVars;
}

unsigned SymbolTable::getNumStaticVars() const {
    return numStaticVars;
}

