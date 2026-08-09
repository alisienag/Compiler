#include "lexer.h"
#include "token.h"

#include <fstream>
#include <iostream>
#include <sstream>

bool isWhitespace(const char);
bool isDigit(const char);
bool isAlpha(const char);
bool isAlphaNum(const char);

std::string eatWord(const std::string& src, const int i, int* steps);
int eatNumber(const std::string& src, const int i,  int* steps);

Token matchKeyword(const std::string& word);
Token matchSymbol(const char first, const char second);

int line_ = 1;

Lexer::Lexer(std::string path) {

    std::ifstream stream(path);
    
    std::string line;
    while (std::getline(stream, line)) {
        this->src_ += line + '\n';
    }

    stream.close();

}

std::vector<Token> Lexer::tokenise() {

    std::vector<Token> tokens;
    
    //PARSE
    for (std::size_t i = 0; i < this->src_.size(); i++) {
        char current = this->src_.at(i);
        if (isWhitespace(current)) { // skip characters we don't care about
            continue;
        }
        // keywords should be checked first, top priority, then identifiers
        if (isAlpha(current)) {
            int steps = 1;
            std::string value = eatWord(this->src_, i, &steps);
            Token token = matchKeyword(value);
            if (token.type == TokenType::Error) {
                //Treat as identifier, call errors in parser
                token.type = TokenType::Identifier;
                int idx = stringtable_.addString(value);
                token.value = idx; 
            }
            tokens.push_back(token);
            i += steps - 1;
            continue;
        }
        // then numbers
        if (isDigit(current)) {
            int steps = 1;
            int value = eatNumber(this->src_, i, &steps);
            Token token;
            token.type = TokenType::Number;
            token.value = value;
            tokens.push_back(token);
            i += steps - 1;
            continue;
        }

        //check symbols here
        char next = ' ';
        if (i + 1 < this->src_.size())
            next = this->src_.at(i+1);
        Token token = matchSymbol(current, next);
        if (token.type == TokenType::Speech) {
            tokens.push_back(token);
            std::string str = "";
            while (next != '\"') {
                str.push_back(next);
                i++;
                next = this->src_.at(i+1);
            }
            Token strToken;
            strToken.line = line_;
            strToken.type = TokenType::String;
            strToken.value = stringtable_.addString(str);
            tokens.push_back(strToken);
            //token already holds speech mark, just push another
            i++;
        }
        tokens.push_back(token);
    }
    
    Token endToken;
    endToken.type = TokenType::End;
    endToken.decimalValue = 0;
    tokens.push_back(endToken);

    return tokens;
}

Token matchKeyword(const std::string& word) {
    Token token;
    token.type = TokenType::Error;
    token.decimalValue = 0;
    token.line = line_;
    if (word.compare("let") == 0) {
        token.type = TokenType::Let;
        return token;
    }  else if (word.compare("i32") == 0) {
        token.type = TokenType::i32;
        return token;
    } else if (word.compare("ret") == 0) {
        token.type = TokenType::Ret;
        return token;
    } else if (word.compare("string") == 0) {
        token.type = TokenType::String;
        return token;
    } else if (word.compare("true") == 0) {
        token.type = TokenType::True;
        token.value = 1;
    } else if (word.compare("false") == 0) {
        token.type = TokenType::False;
        token.value = 0;
    } else if (word.compare("bool") == 0) {
        token.type = TokenType::Bool;
    } else if (word.compare("if") == 0) {
        token.type = TokenType::If;
    } else if (word.compare("else") == 0) {
        token.type = TokenType::Else;
    }
    return token;
}

Token matchSymbol(const char first, const char second) {
    Token token;
    token.type = TokenType::Error;
    token.decimalValue = 0;
    token.line = line_;
    if (first == '+') {
        token.type = TokenType::Plus;
        return token;
    } else if (first == '-') {
        token.type = TokenType::Minus;
        return token;
    } else if (first == '*') {
        token.type = TokenType::Mul;
        return token;
    } else if (first == '/') {
        token.type = TokenType::Div;
        return token;
    } else if (first == '(') {
        token.type = TokenType::LParen;
        return token;
    } else if (first == ')') {
        token.type = TokenType::RParen;
        return token;
    } else if (first == ':') {
        token.type = TokenType::Colon;
        return token;
    } else if (first == '=') {
        if (second == '=') {
            token.type = TokenType::Equal;
            return token;
        }
        token.type = TokenType::Equals;
        return token;
    } else if (first == ';') {
        token.type = TokenType::SColon;
    } else if (first == '>') {
        token.type = TokenType::RArrow;
    } else if (first == '<') {
        token.type = TokenType::LArrow;
    } else if (first == '{') {
        token.type = TokenType::LCurly;
    } else if (first == '}') {
        token.type = TokenType::RCurly;
    } else if (first == ',') {
        token.type = TokenType::Comma;
    } else if (first == '\"') {
        token.type = TokenType::Speech;
    } else if (first == '!') {
        if (second == '=') {
            token.type = TokenType::NEqual;
        }
    }
    return token;
}

std::string eatWord(const std::string& src, const int i, int* steps) {
    std::size_t index = 1;
    std::stringstream stream;
    stream << src.at(i);
    while (i + index < src.size() && isAlphaNum(src.at(i + index))) {
        stream << (src.at(i + index));
        index++;
    }
    *steps = static_cast<int>(index);
    return stream.str();
}

int eatNumber(const std::string& src, const int i,  int* steps) {
    std::size_t index = 1;
    int value = src.at(i) - '0'; // converts char to int
    while (i + index < src.size() && isDigit(src.at(i + index))) {
        value *= 10;
        value += (src.at(i + index) - '0');
        index++;
    }
    *steps = static_cast<int>(index);
    return value;
}

bool isWhitespace(const char a) {
    if (a =='\n')
        line_++;
    return (a == ' ' || a == '\r' || a == '\n');
}
bool isDigit(const char a) {
    return (a >= '0' && a <= '9');
}
bool isAlpha(const char a) {
    return (a >= 'a' && a <= 'z') || (a >= 'A' && a <= 'Z');
}
bool isAlphaNum(const char a) {
    return isDigit(a) || isAlpha(a);
}
