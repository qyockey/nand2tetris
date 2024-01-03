#include <algorithm>
#include <filesystem>
#include <sstream>

#include "CompilationEngine.h"
#include "Token.h"
#include "UnexpectedTokenException.h"

CompilationEngine::CompilationEngine(const fs::path& jackFilePath,
        const fs::path& vmFilePath)
            : tokenizer(jackFilePath), vmWriter(vmFilePath) {}

CompilationEngine::~CompilationEngine() = default;

void CompilationEngine::printJackFilePosition() const {
    std::cout << "Line " << tokenizer.getCurrentLine() << " col "
            << tokenizer.getCurrentCol() << ": ";
}

void CompilationEngine::compileClass() {
    expectKeyword(Token::Keyword::CLASS);
    className = tokenizer.getToken().getValue();
    expectType(Token::TokenType::IDENTIFIER);
    expectSymbol('{');
    while (tokenizer.getToken().matchesKeywords(Token::classVarKinds)) {
        compileClassVarDec();
    }
    while (tokenizer.getToken().matchesKeywords(Token::subroutineTypes)) {
        subroutineVars.reset();
        compileSubroutine();
    }
    expectSymbol('}');
}

void CompilationEngine::compileClassVarDec() {
    const Variable::Kind varKind = Variable::strToKind(
            tokenizer.getToken().getValue());
    expectKeywords(Token::classVarKinds);
    const std::string varType = tokenizer.getToken().getValue();
    compileType();
    std::string varName = tokenizer.getToken().getValue();
    expectType(Token::TokenType::IDENTIFIER);
    classVars.define(varName, varType, varKind);
    while (tokenizer.getToken().getSymbol() == ',') {
        expectSymbol(',');
        varName = tokenizer.getToken().getValue();
        expectType(Token::TokenType::IDENTIFIER);
        classVars.define(varName, varType, varKind);
    }
    expectSymbol(';');
}

void CompilationEngine::compileType() {
    if (tokenizer.getToken().matchesKeywords(Token::primitiveTypes)) {
        expectKeywords(Token::primitiveTypes);
    } else {
        expectType(Token::TokenType::IDENTIFIER);
    }
}

void CompilationEngine::compileSubroutine() {
    const Token::Keyword subroutineType = tokenizer.getToken().getKeyword();
    expectKeywords(Token::subroutineTypes);
    if (tokenizer.getToken().getKeyword() == Token::Keyword::VOID) {
        expectKeyword(Token::Keyword::VOID);
    } else {
        compileType();
    }
    const std::string subroutineName = tokenizer.getToken().getValue();
    const std::string subroutineCall = className + '.' + subroutineName;
    expectType(Token::TokenType::IDENTIFIER);
    if (subroutineType == Token::Keyword::METHOD) {
        subroutineVars.define("this", className, Variable::Kind::ARG);
    }
    expectSymbol('(');
    compileParameterList();
    expectSymbol(')');
    expectSymbol('{');
    while (tokenizer.getToken().getKeyword() == Token::Keyword::VAR) {
        compileVarDec();
    }
    vmWriter.writeFunction(subroutineCall, subroutineVars.getNumLocalVars());
    if (subroutineType == Token::Keyword::CONSTRUCTOR) {
        unsigned numFields = classVars.getNumFields();
        vmWriter.writePush(VmWriter::PushSegment::CONSTANT, numFields);
        vmWriter.writeCall("Memory.alloc", 1);
        vmWriter.writePop(VmWriter::PopSegment::POINTER, 0);
    } else if (subroutineType == Token::Keyword::METHOD) {
        vmWriter.writePush(VmWriter::PushSegment::ARG, 0);
        vmWriter.writePop(VmWriter::PopSegment::POINTER, 0);
    }
    compileStatements();
    expectSymbol('}');
}

void CompilationEngine::compileSubroutineCall() {
    std::string subroutineClass;
    std::string subroutineName;
    std::string subroutineCall;
    bool isMethod;
    const std::string firstIdentifier = tokenizer.getToken().getValue();
    expectType(Token::TokenType::IDENTIFIER);
    if (tokenizer.getToken().getSymbol() == '.') {
        expectSymbol('.');
        subroutineName = tokenizer.getToken().getValue();
        expectType(Token::TokenType::IDENTIFIER);
        try {
            // method in other class
            isMethod = true;
            Variable object = getVariable(firstIdentifier);
            subroutineClass = object.getType();
            vmWriter.writePush(object);
        } catch (const UnexpectedTokenException&) {
            // static function or constructor
            isMethod = false;
            subroutineClass = firstIdentifier;
        }
    } else {
        // method in current class
        isMethod = true;
        vmWriter.writePush(VmWriter::PushSegment::POINTER, 0);
        subroutineClass = className;
        subroutineName = firstIdentifier;
    }
    subroutineCall = subroutineClass + "." + subroutineName;
    expectSymbol('(');
    unsigned numParams = compileExpressionList();
    if (isMethod) {
        numParams++;
    }
    expectSymbol(')');
    vmWriter.writeCall(subroutineCall, numParams);
}

void CompilationEngine::compileParameterList() {
    while (tokenizer.getToken().matchesKeywords(Token::primitiveTypes)
            || tokenizer.getToken().getType() == Token::TokenType::IDENTIFIER) {
        std::string varType = tokenizer.getToken().getValue();
        compileType();
        std::string varName = tokenizer.getToken().getValue();
        expectType(Token::TokenType::IDENTIFIER);
        subroutineVars.define(varName, varType, Variable::Kind::ARG);
        while (tokenizer.getToken().getSymbol() == ',') {
            expectSymbol(',');
            varType = tokenizer.getToken().getValue();
            compileType();
            varName = tokenizer.getToken().getValue();
            expectType(Token::TokenType::IDENTIFIER);
            subroutineVars.define(varName, varType, Variable::Kind::ARG);
        }
    }
}

void CompilationEngine::compileVarDec() {
    expectKeyword(Token::Keyword::VAR);
    const std::string varType = tokenizer.getToken().getValue();
    compileType();
    std::string varName = tokenizer.getToken().getValue();
    expectType(Token::TokenType::IDENTIFIER);
    subroutineVars.define(varName, varType, Variable::Kind::VAR);
    while (tokenizer.getToken().getSymbol() == ',') {
        expectSymbol(',');
        varName = tokenizer.getToken().getValue();
        expectType(Token::TokenType::IDENTIFIER);
        subroutineVars.define(varName, varType, Variable::Kind::VAR);
    }
    expectSymbol(';');
}

void CompilationEngine::compileStatements() {
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
                throw UnexpectedTokenException("Expected statement");
                return;
        }
    }
}

Variable CompilationEngine::getVariable(const std::string& varName) {
    if (subroutineVars.contains(varName)) {
        return subroutineVars.get(varName);
    }
    if (classVars.contains(varName)) {
        return classVars.get(varName);
    }
    throw UnexpectedTokenException("Variable '" + varName + "' not defined");
}

void CompilationEngine::compileDoStatement() {
    expectKeyword(Token::Keyword::DO);
    compileSubroutineCall();
    expectSymbol(';');
    vmWriter.writePop(VmWriter::PopSegment::TEMP, 0);
}

void CompilationEngine::compileLetStatement() {
    expectKeyword(Token::Keyword::LET);
    const std::string varName = tokenizer.getToken().getValue();
    const Variable variable = getVariable(varName);
    expectType(Token::TokenType::IDENTIFIER);
    bool isArray = tokenizer.getToken().getSymbol() == '[';
    if (isArray) {
        expectSymbol('[');
        vmWriter.writePush(variable);
        compileExpression();
        expectSymbol(']');
        vmWriter.writeArithmetic(VmWriter::Command::ADD);
    }
    expectSymbol('=');
    compileExpression();
    if (isArray) {
        vmWriter.writePop(VmWriter::PopSegment::TEMP, 0);
        vmWriter.writePop(VmWriter::PopSegment::POINTER, 1);
        vmWriter.writePush(VmWriter::PushSegment::TEMP, 0);
        vmWriter.writePop(VmWriter::PopSegment::THAT, 0);
    } else {
        vmWriter.writePop(variable);
    }
    expectSymbol(';');
}

void CompilationEngine::compileWhileStatement() {
    static unsigned labelNum = 0;
    std::string labelNumStr = std::to_string(labelNum);
    std::string beginLabel = "BEGIN_WHILE_" + labelNumStr;
    std::string endLabel = "END_WHILE_" + labelNumStr;
    labelNum++;
    expectKeyword(Token::Keyword::WHILE);
    vmWriter.writeLabel(beginLabel);
    expectSymbol('(');
    compileExpression();
    expectSymbol(')');
    vmWriter.writeArithmetic(VmWriter::Command::NOT);
    vmWriter.writeIfGoto(endLabel);
    expectSymbol('{');
    compileStatements();
    expectSymbol('}');
    vmWriter.writeGoto(beginLabel);
    vmWriter.writeLabel(endLabel);
}

void CompilationEngine::compileReturnStatement() {
    expectKeyword(Token::Keyword::RETURN);
    if (tokenizer.getToken().matchesTerm()) {
        compileExpression();
    } else {
        vmWriter.writePush(VmWriter::PushSegment::CONSTANT, 0);
    }
    expectSymbol(';');
    vmWriter.writeReturn();
}

void CompilationEngine::compileIfStatement() {
    static unsigned labelNum = 0;
    std::string labelNumStr = std::to_string(labelNum);
    std::string trueLabel = "IF_TRUE_" + labelNumStr;
    std::string falseLabel = "IF_FALSE_" + labelNumStr;
    std::string endLabel = "END_IF_" + labelNumStr;
    labelNum++;
    expectKeyword(Token::Keyword::IF);
    expectSymbol('(');
    compileExpression();
    expectSymbol(')');
    vmWriter.writeIfGoto(trueLabel);
    vmWriter.writeGoto(falseLabel);
    vmWriter.writeLabel(trueLabel);
    expectSymbol('{');
    compileStatements();
    expectSymbol('}');
    if (tokenizer.getToken().getKeyword() == Token::Keyword::ELSE) {
        vmWriter.writeGoto(endLabel);
        vmWriter.writeLabel(falseLabel);
        expectKeyword(Token::Keyword::ELSE);
        expectSymbol('{');
        compileStatements();
        expectSymbol('}');
        vmWriter.writeLabel(endLabel);
    } else {
        vmWriter.writeLabel(falseLabel);
    }
}

void CompilationEngine::compileTerm() {
    switch (tokenizer.getToken().getType()) {
        case Token::TokenType::IDENTIFIER:
            if (tokenizer.getNextToken().getSymbol() == '[') {
                vmWriter.writePush(getVariable(tokenizer.getToken().getValue()));
                expectType(Token::TokenType::IDENTIFIER);
                expectSymbol('[');
                compileExpression();
                expectSymbol(']');
                vmWriter.writeArithmetic(VmWriter::Command::ADD);
                vmWriter.writePop(VmWriter::PopSegment::POINTER, 1);
                vmWriter.writePush(VmWriter::PushSegment::THAT, 0);
            } else if (const char nextSymbol = 
                        tokenizer.getNextToken().getSymbol();
                    nextSymbol == '.' || nextSymbol == '(') {
                compileSubroutineCall();
            } else {
                vmWriter.writePush(getVariable(
                        tokenizer.getToken().getValue()));
                expectType(Token::TokenType::IDENTIFIER);
            }
            break;
        case Token::TokenType::INT_CONST:
            vmWriter.writePush(VmWriter::PushSegment::CONSTANT,
                    expectIntConstant());
            break;
        case Token::TokenType::STRING_CONST:
            vmWriter.writePush(VmWriter::PushSegment::CONSTANT,
                    (unsigned) tokenizer.getToken().getValue().length());
            vmWriter.writeCall("String.new", 1);
            for (const char c : tokenizer.getToken().getValue()) {
                vmWriter.writePush(VmWriter::PushSegment::CONSTANT,
                        (unsigned) (c));
                vmWriter.writeCall("String.appendChar", 2);
            }
            expectType(Token::TokenType::STRING_CONST);
            break;
        case Token::TokenType::KEYWORD:
            switch (tokenizer.getToken().getKeyword()) {
                case Token::Keyword::TRUE:
                    expectKeyword(Token::Keyword::TRUE);
                    vmWriter.writePush(VmWriter::PushSegment::CONSTANT, 0);
                    vmWriter.writeArithmetic(VmWriter::Command::NOT);
                    break;
                case Token::Keyword::FALSE:
                    expectKeyword(Token::Keyword::FALSE);
                    vmWriter.writePush(VmWriter::PushSegment::CONSTANT, 0);
                    break;
                case Token::Keyword::NULL_:
                    expectKeyword(Token::Keyword::NULL_);
                    vmWriter.writePush(VmWriter::PushSegment::CONSTANT, 0);
                    break;
                case Token::Keyword::THIS:
                    expectKeyword(Token::Keyword::THIS);
                    vmWriter.writePush(VmWriter::PushSegment::POINTER, 0);
                    break;
                default:
                    throw UnexpectedTokenException(
                            R"(Expected "true", "false", "null", or "this")");
            }
            break;
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
                    vmWriter.writeArithmetic(VmWriter::Command::NEG);
                    break;
                case '~':
                    expectSymbol('~');
                    compileTerm();
                    vmWriter.writeArithmetic(VmWriter::Command::NOT);
                    break;
                default:
                    throw UnexpectedTokenException("Expected term");
            }
            break;
        default:
            throw UnexpectedTokenException("Expected term");
    }
}

void CompilationEngine::compileExpression() {
    compileTerm();
    while (tokenizer.getToken().matchesSymbols(Token::binaryOperators)) {
        const char binaryOperator = tokenizer.getToken().getSymbol();
        expectSymbols(Token::binaryOperators);
        compileTerm();
        switch (binaryOperator) {
            case '+':
                vmWriter.writeArithmetic(VmWriter::Command::ADD);
                break;
            case '-':
                vmWriter.writeArithmetic(VmWriter::Command::SUB);
                break;
            case '*':
                vmWriter.writeCall("Math.multiply", 2);
                break;
            case '/':
                vmWriter.writeCall("Math.divide", 2);
                break;
            case '&':
                vmWriter.writeArithmetic(VmWriter::Command::AND);
                break;
            case '|':
                vmWriter.writeArithmetic(VmWriter::Command::OR);
                break;
            case '<':
                vmWriter.writeArithmetic(VmWriter::Command::LT);
                break;
            case '=':
                vmWriter.writeArithmetic(VmWriter::Command::EQ);
                break;
            case '>':
                vmWriter.writeArithmetic(VmWriter::Command::GT);
                break;
            default:
                throw UnexpectedTokenException("Expected binary operator");
        }
    }
}

unsigned CompilationEngine::compileExpressionList() {
    unsigned numExpressions = 0;
    if (tokenizer.getToken().matchesTerm()) {
        compileExpression();
        numExpressions++;
        while (tokenizer.getToken().getSymbol() == ',') {
            expectSymbol(',');
            compileExpression();
            numExpressions++;
        }
    }
    return numExpressions;
}

void CompilationEngine::expectSymbol(char symbol) {
    if (const Token token = tokenizer.getToken();
            token.getType() != Token::TokenType::SYMBOL
                || token.getSymbol() != symbol) {
        throw UnexpectedTokenException("Expected symbol '" + std::string(1, symbol) + "'");
    }
    tokenizer.advance();
}

void CompilationEngine::expectSymbols(const std::vector<char>& symbols) {
    for (const char symbol : symbols) {
        try {
            expectSymbol(symbol);
            return;
        } catch (const UnexpectedTokenException&) {
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
    throw UnexpectedTokenException(errorMessageStream.str());
}

void CompilationEngine::expectKeyword(const Token::Keyword keyword) {
    std::string keywordStr = Token::keywordToStr(keyword);
    if (const Token token = tokenizer.getToken();
            token.getType() != Token::TokenType::KEYWORD
                || token.getKeyword() != keyword) {
        throw UnexpectedTokenException("Expected keyword \"" + keywordStr + "\"");
    }
    tokenizer.advance();
}

void CompilationEngine::expectKeywords(
        const std::vector<Token::Keyword>& keywords) {
    for (auto const& keyword : keywords) {
        try {
            expectKeyword(keyword);
            return;
        } catch (const UnexpectedTokenException&) {
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
    throw UnexpectedTokenException(errorMessageStream.str());
}

unsigned CompilationEngine::expectIntConstant() {
    unsigned value = tokenizer.getToken().getIntValue();
    tokenizer.advance();
    return value;
}

void CompilationEngine::expectType(const Token::TokenType type) {
    if (tokenizer.getToken().getType() != type) {
        throw UnexpectedTokenException("Expected type");
    }
    const std::string tag = Token::tokenTypeToStr(type);
    tokenizer.advance();
}

void CompilationEngine::expectTypes(
        const std::vector<Token::TokenType>& tokenTypes) {
    for (auto const& tokenType : tokenTypes) {
        try {
            expectType(tokenType);
            return;
        } catch (const UnexpectedTokenException&) {
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
    throw UnexpectedTokenException(errorMessageStream.str());
}

