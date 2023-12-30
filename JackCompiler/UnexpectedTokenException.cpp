#include "UnexpectedTokenException.h"

UnexpectedTokenException::UnexpectedTokenException(const char *msg)
        : message(msg) {}

UnexpectedTokenException::UnexpectedTokenException(const std::string &msg)
        : message(msg) {}

const char *UnexpectedTokenException::what() const noexcept {
    return message.c_str();
}

