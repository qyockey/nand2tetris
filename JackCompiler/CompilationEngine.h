#ifndef COMPILATION_ENGINE_H
#define COMPILATION_ENGINE_H

#include <string>
#include <unordered_map>

#include "JackTokenizer.h"
#include "SymbolTable.h"
#include "Variable.h"
#include "VmWriter.h"

namespace fs = std::filesystem;

class CompilationEngine {
public:
    CompilationEngine(const fs::path& jackFilePath,
            const fs::path& vmFilePath);
    ~CompilationEngine();
    void printJackFilePosition() const;
    void compileClass();

private:
    JackTokenizer tokenizer;
    VmWriter vmWriter;
    SymbolTable classVars {};
    SymbolTable subroutineVars {};
    std::string className {};
    void compileClassVarDec();
    void compileType();
    void compileSubroutine();
    void compileSubroutineCall();
    void compileParameterList();
    void compileVarDec();
    void compileStatements();
    void compileDoStatement();
    void compileLetStatement();
    void compileWhileStatement();
    void compileReturnStatement();
    void compileIfStatement();
    void compileTerm();
    void compileExpression();
    unsigned compileExpressionList();

    Variable getVariable(const std::string& varName);
    void expectSymbol(char symbol);
    void expectSymbols(const std::vector<char>& symbols);
    void expectKeyword(const Token::Keyword keyword);
    void expectKeywords(const std::vector<Token::Keyword>& keywords);
    unsigned expectIntConstant();
    void expectType(const Token::TokenType type);
    void expectTypes(const std::vector<Token::TokenType>& tokenTypes);
};

#endif

