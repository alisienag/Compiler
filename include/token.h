#pragma once

#include <cstddef>
enum class TokenType {
    Let,
    Const,
    i32,
    u8,
    Number,
    Plus,
    Minus,
    Mul,
    Div,
    LParen,
    RParen,
    Colon,
    Equals,
    SColon,
    Identifier,
    RArrow,
    LArrow,
    LCurly,
    RCurly,
    Ret,
    Comma,
    Speech,
    String,
    True,
    False,
    Bool,
    Equal,
    NEqual,
    LEqual,
    GEqual,
    If,
    Else,
    LSquare,
    RSquare,
    End,
    Error
};

typedef struct {
    TokenType type;
    union {
        double decimalValue;
        int value;
    };
    size_t line;
} Token;
