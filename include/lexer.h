#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "stringtable.h"
#include "token.h"

class Lexer {
    public:
    explicit Lexer(std::string path);
    std::vector<Token> tokenise();
    StringTable stringtable_;
    private:
    std::string src_;
    std::size_t pos_ = 0;
};
