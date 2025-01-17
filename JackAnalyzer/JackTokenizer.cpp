#include <array>
#include <sstream>

#include "Token.h"
#include "JackTokenizer.h"
#include "UnexpectedTokenException.h"

#define SYMBOL_COUNT 19
#define KEYWORD_COUNT 21

JackTokenizer::JackTokenizer(const fs::path &filePath) : jackFile(filePath) {
    advance();
    advance();
}

JackTokenizer::~JackTokenizer() {
    jackFile.close();
}

bool JackTokenizer::hasMoreTokens() {
    trimWhiteSpaceAndComments();
    return jackFile.peek() != EOF;
}

void JackTokenizer::advance() {
    if (hasMoreTokens()) {
        token.setValue(nextToken.getValue());
        token.setType(nextToken.getType());
        token.setKeyword(nextToken.getKeyword());
        if (auto nextChar = jackFile.peek(); isdigit(nextChar)) {
            tokenizeIntConst();
        } else if (isalpha(nextChar)) {
            tokenizeKeywordOrIdentifier();
        } else if (nextChar == '"') {
            tokenizeStringConst();
        } else {
            tokenizeSymbol();
        }
    }
}

Token JackTokenizer::getToken() const {
    return token;
}

Token JackTokenizer::getNextToken() const {
    return nextToken;
}

int JackTokenizer::getCurrentLine() const {
    return currentLine;
}

char JackTokenizer::getNextChar() {
    auto nextChar = (char) jackFile.get();
    if (nextChar == '\n') {
        currentLine++;
    }
    return nextChar;
}

void JackTokenizer::trimWhiteSpaceAndComments() {
    int nextChar;
    do {
        nextChar = getNextChar();
        if (nextChar == '/' && jackFile.peek() == '/') {
            do {
                nextChar = getNextChar();
            } while (nextChar != '\n');
        } else if (nextChar == '/' && jackFile.peek() == '*') {
            do {
                nextChar = getNextChar();
            } while (!(nextChar == '*' && jackFile.peek() == '/'));
            getNextChar();
            nextChar = getNextChar();
        }
    } while (isspace(nextChar));
    jackFile.unget();
}

void JackTokenizer::tokenizeKeywordOrIdentifier() {
    std::stringstream tokenStream;
    do {
        tokenStream << getNextChar();
    } while (isalnum(jackFile.peek()) || jackFile.peek() == '_');
    std::string str = tokenStream.str();
    Token::Keyword keyword = Token::strToKeyword(str);
    nextToken.setKeyword(keyword);
    if (keyword == Token::Keyword::INVALID) {
        nextToken.setType(Token::TokenType::IDENTIFIER);
    } else {
        nextToken.setType(Token::TokenType::KEYWORD);
    }
    nextToken.setValue(str);
}

void JackTokenizer::tokenizeIntConst() {
    std::stringstream tokenStream;
    nextToken.setType(Token::TokenType::INT_CONST);
    nextToken.setKeyword(Token::Keyword::INVALID);
    while (isdigit(jackFile.peek())) {
        tokenStream << getNextChar();
    }
    int16_t val;
    std::string str = tokenStream.str();
    std::stringstream intValStream(str);
    intValStream >> val;
    if (!intValStream.eof() || intValStream.fail()) {
            throw UnexpectedTokenException("Invalid integer constant '"
                    + str + "'");
    }
    nextToken.setValue(str);
}

void JackTokenizer::tokenizeStringConst() {
    nextToken.setType(Token::TokenType::STRING_CONST);
    nextToken.setKeyword(Token::Keyword::INVALID);
    std::stringstream tokenStream;
    (void) getNextChar();
    char nextChar;
    while ((nextChar = getNextChar()) != '"') {
        tokenStream << nextChar;
    }
    nextToken.setValue(tokenStream.str());
}

void JackTokenizer::tokenizeSymbol() {
    char nextChar = getNextChar();
    std::stringstream tokenStream;
    for (char symbol : Token::allSymbols) {
        if (nextChar == symbol) {
            nextToken.setType(Token::TokenType::SYMBOL);
            nextToken.setKeyword(Token::Keyword::INVALID);
            nextToken.setValue(std::string(1, symbol));
            tokenStream << nextChar;
            return;
        }
    }
    throw UnexpectedTokenException("Invalid symbol '"
            + std::string(1, nextChar) + "'");
}

