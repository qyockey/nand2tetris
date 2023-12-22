#include <sstream>

#include "JackTokenizer.h"

#define SYMBOL_COUNT 19
#define KEYWORD_COUNT 21

JackTokenizer::JackTokenizer(std::string filePath)
{
    jackFile.open(filePath);
}

JackTokenizer::~JackTokenizer()
{
    jackFile.close();
}

bool JackTokenizer::hasMoreTokens()
{
    trimWhiteSpaceAndComments();
    return jackFile.peek() != EOF;
}

void JackTokenizer::advance()
{
    static char symbols[] = {'{', '}', '(', ')', '[', ']', '.', ',', ';', '+',
            '-', '*', '/', '&', '|', '<', '>', '=', '~'};
    static std::string keywords[] = {"class", "constructor", "function",
            "method", "field", "static", "var", "int", "char", "boolean",
            "void", "true", "false", "null", "this", "let", "do", "if",
            "else", "while", "return"};
    std::stringstream tokenStream;
    trimWhiteSpaceAndComments();
    char nextChar = jackFile.get();
    if (isdigit(nextChar)) {
        tokenType = TokenType::INT_CONST;
        tokenStream << nextChar;
        while (isdigit(jackFile.peek())) {
            nextChar = jackFile.get();
            tokenStream << nextChar;
        }
    } else if (isalpha(nextChar)) {
        tokenStream << nextChar;
        while (isalnum(jackFile.peek())) {
            nextChar = jackFile.get();
            tokenStream << nextChar;
        }
        bool validKeyword = false;
        for (int i = 0; i < KEYWORD_COUNT; i++) {
            if (tokenStream.str().compare(keywords[i]) == EXIT_SUCCESS) {
                validKeyword = true;
                tokenType = TokenType::KEYWORD;
                keywordType = (KeywordType) i;
                break;
            }
        }
        if (!validKeyword) {
            tokenType = TokenType::IDENTIFIER;
        }
    } else if (nextChar == '"') {
        tokenType = TokenType::STRING_CONST;
        while ((nextChar = jackFile.get()) != '"') {
            tokenStream << nextChar;
        }
    } else {
        bool validSymbol = false;
        for (int i = 0; i < SYMBOL_COUNT; i++) {
            if (nextChar == symbols[i]) {
                validSymbol = true;
                tokenType = TokenType::SYMBOL;
                tokenStream << nextChar;
                break;
            }
        }
        if (!validSymbol) {
            // error
        }
    }
    token = tokenStream.str();
}

JackTokenizer::TokenType JackTokenizer::getTokenType()
{
    return tokenType;
}

std::string JackTokenizer::keyword()
{
    if (tokenType != TokenType::KEYWORD) {
        // error
    }
    return token;
}

std::string JackTokenizer::symbol()
{
    if (tokenType != TokenType::SYMBOL) {
        // error
    }
    if (token.compare("<") == EXIT_SUCCESS) {
        return "&lt;";
    } else if (token.compare(">") == EXIT_SUCCESS) {
        return "&gt;";
    } else if (token.compare("&") == EXIT_SUCCESS) {
        return "&amp;";
    }
    return token;
}

std::string JackTokenizer::identifier()
{
    if (tokenType != TokenType::IDENTIFIER) {
        // error
    }
    return token;
}

std::string JackTokenizer::intVal()
{
    if (tokenType != TokenType::INT_CONST) {
        // error
    }
    std::stringstream ss(token);
    int16_t val;
    ss >> val;
    if (ss.eof() && !ss.fail()) {
        return token;
    }
    // error
    return std::string();
}

std::string JackTokenizer::stringVal()
{
    if (tokenType != TokenType::STRING_CONST) {
        // error
    }
    return token;
}

void JackTokenizer::trimWhiteSpaceAndComments()
{
    char nextChar;
    do {
        nextChar = jackFile.get();
        if (nextChar == '/') {
            if (jackFile.peek() == '/') {
                do {
                    nextChar = jackFile.get();
                } while (nextChar != '\n');
            } else if (jackFile.peek() == '*') {
                do {
                    nextChar = jackFile.get();
                } while (!(nextChar == '*' && jackFile.peek() == '/'));
                jackFile.get();
                nextChar = jackFile.get();
            }
        }
    } while (isspace(nextChar));
    jackFile.unget();
}

