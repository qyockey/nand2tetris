#ifndef UNEXPECTED_TOKEN_EXCEPTION_H
#define UNEXPECTED_TOKEN_EXCEPTION_H

#include <iostream>

class UnexpectedTokenException : public std::exception {
public:
    explicit UnexpectedTokenException(const char *msg);
    explicit UnexpectedTokenException(const std::string& msg);
    const char *what() const noexcept override;
private:
    const std::string message {};
};

#endif

