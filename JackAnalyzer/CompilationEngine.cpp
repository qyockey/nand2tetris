#include <algorithm>
#include <filesystem>
#include <sstream>

#include "CompilationEngine.h"
#include "Token.h"
#include "UnexpectedTokenException.h"

CompilationEngine::CompilationEngine(const fs::path& jackFilePath,
        const fs::path& xmlFilePath)
            : tokenizer(jackFilePath), xmlFile(xmlFilePath) {
}

CompilationEngine::~CompilationEngine() {
    xmlFile.close();
}

void CompilationEngine::compileClass() {
    writeOpeningTag("class");
    expectKeyword(Token::Keyword::CLASS);
    expectType(Token::TokenType::IDENTIFIER);
    expectSymbol('{');
    while (tokenizer.getToken().matchesKeywords(Token::classVarTypes)) {
        compileClassVarDec();
    }
    while (tokenizer.getToken().matchesKeywords(Token::subroutineTypes)) {
        compileSubroutine();
    }
    expectSymbol('}');
    writeClosingTag("class");
}

void CompilationEngine::compileClassVarDec() {
    writeOpeningTag("classVarDec");
    expectKeywords(Token::classVarTypes);
    compileType();
    expectType(Token::TokenType::IDENTIFIER);
    while (tokenizer.getToken().getSymbol() == ',') {
        expectSymbol(',');
        expectType(Token::TokenType::IDENTIFIER);
    }
    expectSymbol(';');
    writeClosingTag("classVarDec");
}

void CompilationEngine::compileType() {
    if (tokenizer.getToken().matchesKeywords(Token::primitiveTypes)) {
        expectKeywords(Token::primitiveTypes);
    } else {
        expectType(Token::TokenType::IDENTIFIER);
    }
}

void CompilationEngine::compileSubroutine() {
    writeOpeningTag("subroutineDec");
    expectKeywords(Token::subroutineTypes);
    if (tokenizer.getToken().getKeyword() == Token::Keyword::VOID) {
        expectKeyword(Token::Keyword::VOID);
    } else {
        compileType();
    }
    expectType(Token::TokenType::IDENTIFIER);
    expectSymbol('(');
    compileParameterList();
    expectSymbol(')');
    writeOpeningTag("subroutineBody");
    expectSymbol('{');
    while (tokenizer.getToken().getKeyword() == Token::Keyword::VAR) {
        compileVarDec();
    }
    compileStatements();
    expectSymbol('}');
    writeClosingTag("subroutineBody");
    writeClosingTag("subroutineDec");
}

void CompilationEngine::compileSubroutineCall() {
    expectType(Token::TokenType::IDENTIFIER);
    if (tokenizer.getToken().getSymbol() == '.') {
        expectSymbol('.');
        expectType(Token::TokenType::IDENTIFIER);
    }
    expectSymbol('(');
    compileExpressionList();
    expectSymbol(')');
}

void CompilationEngine::compileParameterList() {
    writeOpeningTag("parameterList");
    while (tokenizer.getToken().matchesKeywords(Token::primitiveTypes)
            || tokenizer.getToken().getType() == Token::TokenType::IDENTIFIER) {
        compileType();
        expectType(Token::TokenType::IDENTIFIER);
        while (tokenizer.getToken().getSymbol() == ',') {
            expectSymbol(',');
            compileType();
            expectType(Token::TokenType::IDENTIFIER);
        }
    }
    writeClosingTag("parameterList");
}

void CompilationEngine::compileVarDec() {
    writeOpeningTag("varDec");
    expectKeyword(Token::Keyword::VAR);
    compileType();
    expectType(Token::TokenType::IDENTIFIER);
    while (tokenizer.getToken().getSymbol() == ',') {
        expectSymbol(',');
        expectType(Token::TokenType::IDENTIFIER);
    }
    expectSymbol(';');
    writeClosingTag("varDec");
}

void CompilationEngine::compileStatements() {
    writeOpeningTag("statements");
    while (tokenizer.getToken().getType() == Token::TokenType::KEYWORD) {
        switch (tokenizer.getToken().getKeyword()) {
            case Token::Keyword::LET:
                compileLetStatement();
                break;
            case Token::Keyword::DO:
                compileDoStatement();
                break;
            case Token::Keyword::IF:
                compileIfStatement();
                break;
            case Token::Keyword::WHILE:
                compileWhileStatement();
                break;
            case Token::Keyword::RETURN:
                compileReturnStatement();
                break;
            default:
                error("Expected statement");
                return;
        }
    }
    writeClosingTag("statements");
}

void CompilationEngine::compileDoStatement() {
    writeOpeningTag("doStatement");
    expectKeyword(Token::Keyword::DO);
    compileSubroutineCall();
    expectSymbol(';');
    writeClosingTag("doStatement");
}

void CompilationEngine::compileLetStatement() {
    writeOpeningTag("letStatement");
    expectKeyword(Token::Keyword::LET);
    expectType(Token::TokenType::IDENTIFIER);
    if (tokenizer.getToken().getSymbol() == '[') {
        expectSymbol('[');
        compileExpression();
        expectSymbol(']');
    }
    expectSymbol('=');
    compileExpression();
    expectSymbol(';');
    writeClosingTag("letStatement");
}

void CompilationEngine::compileWhileStatement() {
    writeOpeningTag("whileStatement");
    expectKeyword(Token::Keyword::WHILE);
    expectSymbol('(');
    compileExpression();
    expectSymbol(')');
    expectSymbol('{');
    compileStatements();
    expectSymbol('}');
    writeClosingTag("whileStatement");
}

void CompilationEngine::compileReturnStatement() {
    writeOpeningTag("returnStatement");
    expectKeyword(Token::Keyword::RETURN);
    if (tokenizer.getToken().matchesTerm()) {
        compileExpression();
    }
    expectSymbol(';');
    writeClosingTag("returnStatement");
}

void CompilationEngine::compileIfStatement() {
    writeOpeningTag("ifStatement");
    expectKeyword(Token::Keyword::IF);
    expectSymbol('(');
    compileExpression();
    expectSymbol(')');
    expectSymbol('{');
    compileStatements();
    expectSymbol('}');
    if (tokenizer.getToken().getKeyword() == Token::Keyword::ELSE) {
        expectKeyword(Token::Keyword::ELSE);
        expectSymbol('{');
        compileStatements();
        expectSymbol('}');
    }
    writeClosingTag("ifStatement");
}

void CompilationEngine::compileTerm() {
    writeOpeningTag("term");
    switch (tokenizer.getToken().getType()) {
        case Token::TokenType::IDENTIFIER:
            if (tokenizer.getNextToken().getSymbol() == '[') {
                expectType(Token::TokenType::IDENTIFIER);
                expectSymbol('[');
                compileExpression();
                expectSymbol(']');
            } else if (tokenizer.getNextToken().getSymbol() == '.') {
                compileSubroutineCall();
            } else {
                expectType(Token::TokenType::IDENTIFIER);
            }
            break;
        case Token::TokenType::INT_CONST:
            expectType(Token::TokenType::INT_CONST);
            break;
        case Token::TokenType::STRING_CONST:
            expectType(Token::TokenType::STRING_CONST);
            break;
        case Token::TokenType::KEYWORD:
            if (tokenizer.getToken().matchesKeywords(Token::keywordConstants)) {
                expectKeywords(Token::keywordConstants);
                break;
            }
            error(R"(Expected "true", "false", "null", or "this")");
        case Token::TokenType::SYMBOL:
            switch (tokenizer.getToken().getSymbol()) {
                case '(':
                    expectSymbol('(');
                    compileExpression();
                    expectSymbol(')');
                    break;
                case '-':
                    expectSymbol('-');
                    compileTerm();
                    break;
                case '~':
                    expectSymbol('~');
                    compileTerm();
                    break;
                default:
                    error("Expected term");
            }
            break;
        default:
            error("Expected term");
    }
    writeClosingTag("term");
}

void CompilationEngine::compileExpression() {
    writeOpeningTag("expression");
    compileTerm();
    while (tokenizer.getToken().matchesSymbols(Token::binaryOperators)) {
        expectSymbols(Token::binaryOperators);
        compileTerm();
    }
    writeClosingTag("expression");
}

void CompilationEngine::compileExpressionList() {
    writeOpeningTag("expressionList");
    if (tokenizer.getToken().matchesTerm()) {
        compileExpression();
        while (tokenizer.getToken().getSymbol() == ',') {
            expectSymbol(',');
            compileExpression();
        }
    }
    writeClosingTag("expressionList");
}

void CompilationEngine::expectSymbol(char symbol) {
    if (const Token token = tokenizer.getToken();
            token.getType() != Token::TokenType::SYMBOL
                || token.getSymbol() != symbol) {
        error("Expected symbol '" + std::string(1, symbol) + "'");
    }
    writeIndent();
    xmlFile << "<symbol> " << Token::symbolToStr(symbol) << " </symbol>\n";
    tokenizer.advance();
}

void CompilationEngine::expectSymbols(const std::vector<char>& symbols) {
    for (const char symbol : symbols) {
        try {
            expectSymbol(symbol);
            return;
        } catch (const UnexpectedTokenException& e) {
            (void) e;
            continue;
        }
    }
    std::stringstream errorMessageStream("");
    errorMessageStream << "Expected one of [";
    for (const char symbol : symbols) {
        errorMessageStream << "'" << symbol << "'";
        if (symbol != symbols.back()) {
            errorMessageStream << ", ";
        }
    }
    errorMessageStream << "]";
    error(errorMessageStream.str());
}

void CompilationEngine::expectKeyword(const Token::Keyword keyword) {
    std::string keywordStr = Token::keywordToStr(keyword);
    if (const Token token = tokenizer.getToken();
            token.getType() != Token::TokenType::KEYWORD
                || token.getKeyword() != keyword) {
        error("Expected keyword \"" + keywordStr + "\"");
    }
    writeIndent();
    xmlFile << "<keyword> " << keywordStr << " </keyword>\n";
    tokenizer.advance();
}

void CompilationEngine::expectKeywords(const std::vector<Token::Keyword>& keywords) {
    for (auto const& keyword : keywords) {
        try {
            expectKeyword(keyword);
            return;
        } catch (const UnexpectedTokenException& e) {
            (void) e;
            continue;
        }
    }
    std::stringstream errorMessageStream("");
    errorMessageStream << "Expected one of [";
    for (auto const& keyword : keywords) {
        errorMessageStream << Token::keywordToStr(keyword);
        if (keyword != keywords.back()) {
            errorMessageStream << ", ";
        }
    }
    errorMessageStream << "]";
    error(errorMessageStream.str());
}

void CompilationEngine::expectType(const Token::TokenType type) {
    if (tokenizer.getToken().getType() != type) {
        error("Expected type");
    }
    const std::string tag = Token::tokenTypeToStr(type);
    writeIndent();
    xmlFile << "<" << tag << "> " << tokenizer.getToken().getValue()
            << " </" << tag << ">\n";
    tokenizer.advance();
}

void CompilationEngine::expectTypes(const std::vector<Token::TokenType>& tokenTypes) {
    for (auto const& tokenType : tokenTypes) {
        try {
            expectType(tokenType);
            return;
        } catch (const UnexpectedTokenException& e) {
            (void) e;
            continue;
        }
    }
    std::stringstream errorMessageStream("");
    errorMessageStream << "Expected one of [";
    for (auto const& tokenType : tokenTypes) {
        errorMessageStream << Token::tokenTypeToStr(tokenType);
        if (tokenType != tokenTypes.back()) {
            errorMessageStream << ", ";
        }
    }
    errorMessageStream << "]";
    error(errorMessageStream.str());
}

void CompilationEngine::writeIndent() {
    for (int i = 0; i < indentLevel; i += 2) {
        xmlFile << "  ";
    }
}

void CompilationEngine::writeOpeningTag(const std::string &tag) {
    writeIndent();
    xmlFile << "<" << tag << ">\n";
    indentLevel += 2;
}

void CompilationEngine::writeClosingTag(const std::string &tag) {
    indentLevel -= 2;
    writeIndent();
    xmlFile << "</" << tag << ">\n";
}

__attribute__((noreturn))
void CompilationEngine::error(const std::string& message) const {
    const std::string errorMessage = "Line "
            + std::to_string(tokenizer.getCurrentLine()) + ": " + message + "\n";
    throw UnexpectedTokenException(errorMessage);
}

