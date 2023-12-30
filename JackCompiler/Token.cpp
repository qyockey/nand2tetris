#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include "Token.h"
#include "UnexpectedTokenException.h"

const std::vector<char> Token::allSymbols {
    '{', '}', '(', ')', '[', ']', '.', ',', ';', '+', '-', '*', '/', '&', '|',
    '<', '>', '=', '~'
};

const std::vector<char> Token::binaryOperators {
    '+', '-', '*', '/', '&', '|', '<', '>', '='
};

const std::vector<char> Token::unaryOperators {
    '-', '~'
};

const std::vector<Token::Keyword> Token::classVarKinds {
    Keyword::STATIC, Keyword::FIELD
};

const std::vector<Token::Keyword> Token::keywordConstants {
    Keyword::TRUE, Keyword::FALSE, Keyword::NULL_, Keyword::THIS
};

const std::vector<Token::Keyword> Token::primitiveTypes {
    Keyword::BOOLEAN, Keyword::CHAR, Keyword::INT
};

const std::vector<Token::Keyword> Token::subroutineTypes {
    Keyword::CONSTRUCTOR, Keyword::FUNCTION, Keyword::METHOD
};

std::string Token::symbolToStr(char symbol) {
    switch (symbol) {
        case '<': return "&lt;";
        case '>': return "&gt;";
        case '&': return "&amp;";
        default: return std::string(1, symbol);
    }
}

Token::Keyword Token::strToKeyword(const std::string& keywordStr) {
    static std::unordered_map<std::string, Keyword> strToKeyword = {
        {"class", Keyword::CLASS},
        {"constructor", Keyword::CONSTRUCTOR},
        {"function", Keyword::FUNCTION},
        {"method", Keyword::METHOD},
        {"field", Keyword::FIELD},
        {"static", Keyword::STATIC},
        {"var", Keyword::VAR},
        {"int", Keyword::INT},
        {"char", Keyword::CHAR},
        {"boolean", Keyword::BOOLEAN},
        {"void", Keyword::VOID},
        {"true", Keyword::TRUE},
        {"false", Keyword::FALSE},
        {"null", Keyword::NULL_},
        {"this", Keyword::THIS},
        {"let", Keyword::LET},
        {"do", Keyword::DO},
        {"if", Keyword::IF},
        {"else", Keyword::ELSE},
        {"while", Keyword::WHILE},
        {"return", Keyword::RETURN}
    };
    try {
        return strToKeyword.at(keywordStr);
    } catch (std::out_of_range&) {
        return Keyword::INVALID;
    }
}

std::string Token::keywordToStr(const Token::Keyword keyword) {
    static std::unordered_map<Keyword, std::string> keywordToStr = {
        {Keyword::CLASS, "class"},
        {Keyword::CONSTRUCTOR, "constructor"},
        {Keyword::FUNCTION, "function"},
        {Keyword::METHOD, "method"},
        {Keyword::FIELD, "field"},
        {Keyword::STATIC, "static"},
        {Keyword::VAR, "var"},
        {Keyword::INT, "int"},
        {Keyword::CHAR, "char"},
        {Keyword::BOOLEAN, "boolean"},
        {Keyword::VOID, "void"},
        {Keyword::TRUE, "true"},
        {Keyword::FALSE, "false"},
        {Keyword::NULL_, "null"},
        {Keyword::THIS, "this"},
        {Keyword::LET, "let"},
        {Keyword::DO, "do"},
        {Keyword::IF, "if"},
        {Keyword::ELSE, "else"},
        {Keyword::WHILE, "while"},
        {Keyword::RETURN, "return"},
    };
    try {
        return keywordToStr.at(keyword);
    } catch (const std::out_of_range&) {
        return std::string("invalid");
    }
}

std::string Token::tokenTypeToStr(const Token::TokenType tokenType) {
    static std::unordered_map<Token::TokenType, std::string> tokenTypeToStr = {
        {Token::TokenType::KEYWORD, "keyword"},
        {Token::TokenType::SYMBOL, "symbol"},
        {Token::TokenType::IDENTIFIER, "identifier"},
        {Token::TokenType::INT_CONST, "integerConstant"},
        {Token::TokenType::STRING_CONST, "stringConstant"}
    };
    try {
        return tokenTypeToStr.at(tokenType);
    } catch (const std::out_of_range&) {
        return std::string("invalid");
    }
}

Token::Token() = default;

Token::Token(const std::string &value, TokenType tokenType)
        : value(value), type(tokenType) {
}

Token::Token(const std::string &value, TokenType tokenType, Keyword keyword)
        : value(value), type(tokenType), keyword(keyword) {
}

std::string Token::getValue() const {
    return value;
}

unsigned Token::getIntValue() const {
    unsigned intVal = -1;
    std::stringstream tokenStream(value);
    tokenStream >> intVal;
    if (tokenStream.eof() && !tokenStream.fail() && intVal != -1U) {
        return intVal;
    } else {
        throw UnexpectedTokenException("Unable to convert '" + value
                + "' to unsigned integer");
    }
}

char Token::getSymbol() const {
    return value[0];
}

Token::TokenType Token::getType() const {
    return type;
}

Token::Keyword Token::getKeyword() const {
    return keyword;
}

void Token::setValue(const std::string_view& newValue){
    value = newValue;
}

void Token::setType(const TokenType newType) {
    type = newType;
}

void Token::setKeyword(const Keyword newKeyword) {
    keyword = newKeyword;
}

bool Token::matchesTypes(const std::vector<TokenType>& types) const {
    return std::any_of(types.begin(), types.end(),
            [this](const TokenType& testType) {
                return testType == type;
    });
}

bool Token::matchesKeywords(const std::vector<Keyword>& keywords) const {
    return std::any_of(keywords.begin(), keywords.end(),
            [this](const Keyword& testKeyword) {
                return testKeyword == keyword;
    });
}

bool Token::matchesSymbols(const std::vector<char>& symbols) const {
    return std::any_of(symbols.begin(), symbols.end(),
            [this](const char& symbol) {
                return symbol == getSymbol();
    });
}

bool Token::matchesTerm() const {
    return matchesTypes(std::vector<Token::TokenType> {
                    Token::TokenType::IDENTIFIER,
                    Token::TokenType::INT_CONST,
                    Token::TokenType::STRING_CONST
                })
            || matchesKeywords(Token::keywordConstants)
            || matchesSymbols(std::vector<char> {'(', '-', '~'});
}

