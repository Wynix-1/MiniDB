#pragma once

#include "string.hpp"
#include <exception>

namespace mini_db {

class Exception : public std::exception {
public:
    Exception(const String& message) : message_(message) {}
    Exception(const char* message) : message_(message) {}

    const char* what() const noexcept override {
        return message_.c_str();
    }

    const String& message() const noexcept {
        return message_;
    }

private:
    String message_;
};

} // namespace mini_db
