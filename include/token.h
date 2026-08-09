#pragma once

#include <cstddef>
enum class TokenType {
    Let, i32, Number, Plus, Minus, Mul, Div, LParen, RParen, Colon, Equals, SColon, Identifier, RArrow, LArrow, LCurly, RCurly, Ret, Comma, Speech, String, True, False, Bool,Equal, NEqual, Less, Greater, LEqual, GEqual, If, Else, End, Error
};

typedef struct {
    TokenType type;
    union {
        double decimalValue;
        int value;
    };
    size_t line;
} Token;
