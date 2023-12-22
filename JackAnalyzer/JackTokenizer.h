#ifndef JACK_TOKENIZER_H
#define JACK_TOKENIZER_H

#include <iostream>
#include <fstream>
#include <vector>

class JackTokenizer {
public:
    enum class TokenType {KEYWORD, SYMBOL, IDENTIFIER, INT_CONST, STRING_CONST};
    enum class KeywordType {CLASS, CONSTRUCTOR, FUNCTION, METHOD, FIELD, STATIC,
            VAR, INT, CHAR, BOOLEAN, VOID, TRUE, FALSE, NULL_, THIS, LET, DO,
            IF, ELSE, WHILE, RETURN};
    explicit JackTokenizer(std::string filePath);
    ~JackTokenizer();
    bool hasMoreTokens();
    void advance();
    TokenType getTokenType();
    std::string keyword();
    std::string symbol();
    std::string identifier();
    std::string intVal();
    std::string stringVal();

private:
    std::ifstream jackFile;
    std::string token;
    TokenType tokenType;
    KeywordType keywordType;
    void trimWhiteSpaceAndComments();
    
};

#endif

