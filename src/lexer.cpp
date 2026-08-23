#include "lexer.h"
#include "token.h"

#include <fstream>
#include <sstream>

bool isWhitespace(const char);
bool isDigit(const char);
bool isAlpha(const char);
bool isAlphaNum(const char);

std::string eatWord(const std::string& src, const int i, int* steps);
int eatNumber(const std::string& src, const int i,  int* steps);

Token matchKeyword(const std::string& word);
Token matchSymbol(const char first, const char second, std::size_t& i);

unsigned int line_ = 1;
unsigned int col_ = 1;

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
                token.strIdx = idx; 
                token.text = value;
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
            token.line = line_;
            token.col = col_;
            token.length = value % 10;
            token.intValue = value;
            tokens.push_back(token);
            i += steps - 1;
            continue;
        }

        //check symbols here
        char next = ' ';
        if (i + 1 < this->src_.size())
            next = this->src_.at(i+1);
        Token token = matchSymbol(current, next, i);
        if (token.type == TokenType::Comment) {
            while (next != '\n') {
                i++;
                next = this->src_.at(i+1 < this->src_.size() ? i + 1 : i);
            }
            continue;
        }
        if (token.type == TokenType::Speech) {
            std::string str = "";
            while (next != '\"') {
                str.push_back(next);
                i++;
                col_++;
                next = this->src_.at(i+1);
            }
            Token strToken;
            strToken.line = line_;
            strToken.col = col_;
            strToken.length = str.length();
            strToken.type = TokenType::String;
            strToken.strIdx = stringtable_.addString(str);
            strToken.text = str;
            tokens.push_back(strToken);
            //token already holds speech mark, just push another
            i++;
            continue;
        } else if (token.type == TokenType::Quote) {
            Token charToken;
            charToken.type = TokenType::Char;
            charToken.line = line_;
            charToken.col = col_;
            charToken.text = std::to_string(next);
            charToken.intValue = next;
            tokens.push_back(charToken);
            i++;
            continue;
        }
        tokens.push_back(token);
    }
    
    Token endToken;
    endToken.type = TokenType::End;
    endToken.line = line_;
    endToken.col = col_;
    tokens.push_back(endToken);

    return tokens;
}

Token matchKeyword(const std::string& word) {
    Token token;
    token.type = TokenType::Error;
    token.line = line_;
    token.col = col_;
    if (word.compare("fn") == 0) {
        token.type = TokenType::Fn;
    }  else if (word.compare("let") == 0) {
        token.type = TokenType::Let;
    } else if (word.compare("mut") == 0) {
        token.type = TokenType::Mut;
    } else if (word.compare("ret") == 0) {
        token.type = TokenType::Ret;
    } else if (word.compare("if") == 0) {
        token.type = TokenType::If;
    } else if (word.compare("else") == 0) {
        token.type = TokenType::Else;
    } else if (word.compare("while") == 0) {
        token.type = TokenType::While;
    } else if (word.compare("break") == 0) {
        token.type = TokenType::Break;
    } else if (word.compare("continue") == 0) {
        token.type = TokenType::Continue;
    } else if (word.compare("as") == 0) {
        token.type = TokenType::As;
    } else if (word.compare("true") == 0) {
        token.type = TokenType::True;
    } else if (word.compare("false") == 0) {
        token.type = TokenType::False;
    } else if (word.compare("i64") == 0) {
        token.type = TokenType::I64;
    } else if (word.compare("u64") == 0) {
        token.type = TokenType::U64;
    } else if (word.compare("i32") == 0) {
        token.type = TokenType::I32;
    } else if (word.compare("u32") == 0) {
        token.type = TokenType::U32;
    } else if (word.compare("i16") == 0) {
        token.type = TokenType::I16;
    } else if (word.compare("u16") == 0) {
        token.type = TokenType::U16;
    } else if (word.compare("i8") == 0) {
        token.type = TokenType::I8;
    } else if (word.compare("u8") == 0) {
        token.type = TokenType::U8;
    } else if (word.compare("bool") == 0) {
        token.type = TokenType::Bool;
    } else if (word.compare("void") == 0) {
        token.type = TokenType::Void;
    }
    token.text = word;
    return token;
}

Token matchSymbol(const char first, const char second, std::size_t& i) {
    Token token;
    token.type = TokenType::Error;
    token.line = line_;
    token.col = col_;
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
        if (second == '/') {
            token.type = TokenType::Comment;
            i++;
            return token;
        }
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
            i++;
            return token;
        }
        token.type = TokenType::Equals;
        return token;
    } else if (first == ';') {
        token.type = TokenType::SColon;
    } else if (first == '>') {
        if (second == '=') {
            token.type = TokenType::GEqual;
            i++;
            return token;
        }
        token.type = TokenType::RArrow;
    } else if (first == '<') {
        if (second == '=') {
            token.type = TokenType::LEqual;
            i++;
            return token;
        }
        token.type = TokenType::LArrow;
    } else if (first == '{') {
        token.type = TokenType::LCurly;
    } else if (first == '}') {
        token.type = TokenType::RCurly;
    } else if (first == ',') {
        token.type = TokenType::Comma;
    } else if (first == '\'') {
        token.type = TokenType::Quote;
    } else if (first == '\"') {
        token.type = TokenType::Speech;
    } else if (first == '!') {
        if (second == '=') {
            token.type = TokenType::NEqual;
            i++;
            return token;
        }
        token.type = TokenType::Exclam;
    } else if (first == '[') {
        token.type = TokenType::LSquare;
    } else if (first == ']') {
        token.type = TokenType::RSquare;
    } else if (first == '&') {
        if (second == '&') {
            token.type = TokenType::AndAnd;
            i++;
            return token;
        }
        token.type = TokenType::And;
    } else if (first == '|') {
        if (second == '|') {
            token.type = TokenType::OrOr;
            i++;
            return token;
        }
        token.type = TokenType::Or;
    }
    return token;
}

std::string eatWord(const std::string& src, const int i, int* steps) {
    std::size_t index = 1;
    std::stringstream stream;
    stream << src.at(i);
    while (i + index < src.size() && isAlphaNum(src.at(i + index))) {
        col_++;
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
        col_++;
        value *= 10;
        value += (src.at(i + index) - '0');
        index++;
    }
    *steps = static_cast<int>(index);
    return value;
}

bool isWhitespace(const char a) {
    if (a =='\n') {
        line_++;
        col_ = 1;
    }
    return (a == ' ' || a == '\r' || a == '\n' || a == '\t');
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
