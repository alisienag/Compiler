#pragma once

#include <string>

enum class TokenType {
    Fn,
    Let,
    Mut,
    Ret,
    If,
    Else,
    While,
    Break,
    Continue,
    As,
    True,
    False,
    I64,
    U64,
    I32,
    U32,
    I16,
    U16,
    I8,
    U8,
    Bool,
    Void,
    Int,
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
    String,
    Number,
    Char,
    RArrow,
    LArrow,
    LCurly,
    RCurly,
    Comma,
    Quote,
    Speech,
    Equal,
    NEqual,
    LEqual,
    GEqual,
    LSquare,
    RSquare,
    And,
    Or,
    AndAnd,
    OrOr,
    Exclam,
    End,
    Comment,
    Error
};

struct Token {
    TokenType type = TokenType::Error;
    long long intValue = 0;
    int strIdx = -1;
    std::string text;
    unsigned int line;
    unsigned int col;
    unsigned int length;
};

inline const std::string tokenName(TokenType t) {
    switch(t) {
        case TokenType::Fn: return "fn";
        case TokenType::Let: return "let";
        case TokenType::Mut: return "mut";
        case TokenType::If: return "if";
        case TokenType::Else: return "else";
        case TokenType::While: return "while";
        case TokenType::U8: return "u8"; // 19

        case TokenType::Identifier: return "ident"; // 32
                                                case TokenType::LSquare: return "["; //47

        case TokenType::Error: return "error";
        default: return std::to_string((int)t);
    }
}
