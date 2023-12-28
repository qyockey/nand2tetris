#ifndef COMPILATION_ENGINE_H
#define COMPILATION_ENGINE_H

#include <string>

#include "JackTokenizer.h"

namespace fs = std::filesystem;

class CompilationEngine {
public:
    CompilationEngine(const fs::path& jackFilePath,
            const fs::path& xmlFilePath);
    ~CompilationEngine();
    void compileClass();

private:
    JackTokenizer tokenizer;
    std::ofstream xmlFile;
    int indentLevel = 0;
    enum class LexicalElement {
        CLASS,
        CLASS_VAR_DEC,
        TYPE,
        SUBROUTINE_DEC,
        PARAMETER_LIST,
        SUBROUTINE_BODY,
        VAR_DEC,
        CLASS_NAME,
        SUBROUTINE_NAME,
        VAR_NAME,
        STATEMENTS,
        STATEMENT,
        LET_STATEMENT,
        IF_STATEMENT,
        WHILE_STATEMENT,
        DO_STATEMENT,
        RETURN_STATEMENT,
        EXPRESSION,
        TERM,
        SUBROUTINE_CALL,
        EXPRESSION_LIST,
        BINARY_OPERATOR,
        UNARY_OPERATOR,
        KEYWORD_CONSTANT
    };
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
    void compileExpressionList();

    void expectSymbol(char symbol);
    void expectSymbols(const std::vector<char>& symbols);
    void expectKeyword(const Token::Keyword keyword);
    void expectKeywords(const std::vector<Token::Keyword>& keywords);
    void expectType(const Token::TokenType type);
    void expectTypes(const std::vector<Token::TokenType>& tokenTypes);
    void expectToken(const std::vector<Token::TokenType>& types,
            const std::vector<Token::Keyword>& keywords,
            const std::vector<char>& symbols);
    void writeIndent();
    void writeOpeningTag(const std::string& tag);
    void writeClosingTag(const std::string& tag);
    void error(const std::string& message) const;
};

#endif

