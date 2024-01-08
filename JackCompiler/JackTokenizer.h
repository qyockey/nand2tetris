#ifndef JACK_TOKENIZER_H
#define JACK_TOKENIZER_H

#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>

#include "Token.h"

namespace fs = std::filesystem;

class JackTokenizer {
public:
    explicit JackTokenizer(const fs::path& filePath);
    ~JackTokenizer();
    bool hasMoreTokens();
    void advance();
    Token getToken() const;
    Token getNextToken() const;
    int getCurrentLine() const;
    int getCurrentCol() const;

private:
    std::ifstream jackFile;
    Token token;
    Token nextToken;
    int currentLine {1};
    int currentCol {1};
    char getNextChar();
    void trimWhiteSpaceAndComments();
    void trimInlineComment();
    void trimMultiLineComment();
    void tokenizeKeywordOrIdentifier();
    void tokenizeIntConst();
    void tokenizeStringConst();
    void tokenizeSymbol();
};

#endif

