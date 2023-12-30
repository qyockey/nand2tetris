#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <vector>

class Token {

public:
    enum class TokenType {
            KEYWORD, SYMBOL, IDENTIFIER, INT_CONST, STRING_CONST, INVALID
    };
    enum class Keyword {
            CLASS, CONSTRUCTOR, FUNCTION, METHOD, FIELD, STATIC, VAR, INT, CHAR,
            BOOLEAN, VOID, TRUE, FALSE, NULL_, THIS, LET, DO, IF, ELSE, WHILE,
            RETURN, INVALID
    };
    static const std::vector<char> allSymbols;
    static const std::vector<char> binaryOperators;
    static const std::vector<char> unaryOperators;
    static const std::vector<Keyword> classVarKinds;
    static const std::vector<Keyword> keywordConstants;
    static const std::vector<Keyword> primitiveTypes;
    static const std::vector<Keyword> subroutineTypes;
    static std::string symbolToStr(char symbol);
    static Keyword strToKeyword(const std::string& keywordStr);
    static std::string keywordToStr(const Token::Keyword keyword);
    static std::string tokenTypeToStr(const Token::TokenType tokenType);
    Token();
    Token(const std::string& value, TokenType tokenType);
    Token(const std::string& value, TokenType tokenType, Keyword keyword);
    std::string getValue() const;
    unsigned getIntValue() const;
    char getSymbol() const;
    TokenType getType() const;
    Keyword getKeyword() const;
    void setValue(const std::string_view& value);
    void setType(const TokenType type);
    void setKeyword(const Keyword keyword);
    bool matchesTypes(const std::vector<TokenType> &types) const;
    bool matchesKeywords(const std::vector<Keyword> &keywords) const;
    bool matchesSymbols(const std::vector<char> &symbols) const;
    bool matchesTerm() const;

private:
    std::string value;
    TokenType type = TokenType::INVALID;
    Keyword keyword = Keyword::INVALID;
};

#endif

