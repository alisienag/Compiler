#include "parser.h"
#include "ast.h"
#include <cstddef>

std::unique_ptr<Program> Parser::parseProgram() {
    auto prog = std::make_unique<Program>();
    size_t start = pos_;
    while (!atEnd()) {
        try {
            prog->functions.push_back(parseFunction());
        } catch (int c) {
            fix();
        }
    }
    prog->span = spanFrom(start);
    return prog;
}

std::unique_ptr<FunctionDecl> Parser::parseFunction() {
    size_t start = pos_;
    expect(TokenType::Fn, "'fn'");
    const Token& name = expect(TokenType::Identifier, "function name");
    
    auto fn = std::make_unique<FunctionDecl>();
    fn->nameIdx = strings_.addString(name.text);
    fn->asmName = name.text;
    expect(TokenType::LParen, "'('");
    if (!check(TokenType::RParen)) {
        do {
            fn->params.push_back(parseParam());
        } while (match(TokenType::Comma));
    }
    expect(TokenType::RParen, "')'");
    expect(TokenType::Colon, "':' before the return type");
    fn->returnType = parseType();
    
    expect(TokenType::Equals, "'{' or '=>'");
    expect(TokenType::RArrow, "'{' or '=>'");

    if (check(TokenType::LCurly)) {
        fn->body = parseBlock();
    } else {
        auto value = parseExpression();
        Span s = spanFrom(start);
        expect(TokenType::SColon, "';' expected after declaring function as expression!");
        auto ret = std::make_unique<ReturnStmt>(std::move(value));
        ret->span = s;
        std::vector<StmtPtr> body;
        body.push_back(std::move(ret));
        fn->body = std::make_unique<BlockStmt>(std::move(body));
        fn->body->span = s;
    }
    fn->span = spanFrom(start);
    return fn;
}

std::unique_ptr<Param> Parser::parseParam() {
    size_t start = pos_;
    bool isMut = match(TokenType::Mut);
    const Token& name = expect(TokenType::Identifier, "parameter name");
    expect(TokenType::Colon, "':' after parameter name");
    Type t = parseType();
    auto p = std::make_unique<Param>(strings_.addString(name.text), t, isMut);
    p->span = spanFrom(start);
    return p;
}

// Statements

StmtPtr Parser::parseStatement() {
    switch(peek().type) {
        case TokenType::Let:
        case TokenType::Mut: return parseLet();
        case TokenType::Ret: return parseReturn();
        case TokenType::If: return parseIf();
        case TokenType::While: return parseWhile();
        case TokenType::LCurly: return parseBlock();
        case TokenType::Break: {
            size_t start = pos_; advance();
            expect(TokenType::SColon, "';' after break");
            auto s = std::make_unique<BreakStmt>();
            s->span = spanFrom(start);
            return s;
        }
        case TokenType::Continue: {
            size_t start = pos_; advance();
            expect(TokenType::SColon, "';' after continue");
            auto s = std::make_unique<ContinueStmt>();
            s->span = spanFrom(start);
            return s;
        }
        default: return parseExprStatement();
    }
}

StmtPtr Parser::parseLet() {
    size_t start = pos_;
    bool isMut = match(TokenType::Mut);
    if (!isMut) {
        expect(TokenType::Let, "'let' or 'mut'");
    }
    const Token& name = expect(TokenType::Identifier, "variable name");
    Type declared = Type::unresolved();
    if (match(TokenType::Colon)) declared = parseType();
    expect(TokenType::Equals, "= after variable declaration");
    auto init = parseExpression();
    expect(TokenType::SColon, "';' after variable initialiser");
    auto s = std::make_unique<LetStmt>(strings_.addString(name.text), declared, std::move(init), isMut);
    s->span = spanFrom(start);
    return s;
}

StmtPtr Parser::parseReturn() {
    size_t start = pos_;
    expect(TokenType::Ret, "'ret'");
    ExprPtr value;
    if (!check(TokenType::SColon)) value = parseExpression();
    expect(TokenType::SColon, "';'");
    auto s = std::make_unique<ReturnStmt>(std::move(value));
    s->span = spanFrom(start);
    return s;
}

StmtPtr Parser::parseIf() {
    size_t start = pos_;
    expect(TokenType::If, "'if'");
    auto cond = parseExpression();
    auto thenBlock = parseBlock();
    StmtPtr elseBranch;
    if (match(TokenType::Else)) {
        if (check(TokenType::If)) {
            elseBranch = parseIf();
        } else if (check(TokenType::LCurly)) {
            elseBranch = parseBlock();
        } else {
            fail(peek(), "expected 'if' or '{' after else");
        }
    }
    auto s = std::make_unique<IfStmt>(std::move(cond), std::move(thenBlock),
            std::move(elseBranch));
    s->span = spanFrom(start);
    return s;
}

StmtPtr Parser::parseWhile() {
    size_t start = pos_;
    expect(TokenType::While, "'while'");
    auto cond = parseExpression();
    auto body = parseBlock();
    auto s = std::make_unique<WhileStmt>(std::move(cond), std::move(body));
    s->span = spanFrom(start);
    return s;
}

std::unique_ptr<BlockStmt> Parser::parseBlock() {
   size_t start = pos_;
   expect(TokenType::LCurly, "'{'");
   std::vector<StmtPtr> stmts;
   while (!check(TokenType::RCurly) && !atEnd()) {
        try {
            stmts.push_back(parseStatement());
        } catch(int i) {
            fix();
        }
   }
   expect(TokenType::RCurly, "'}'");
   auto b = std::make_unique<BlockStmt>(std::move(stmts));
   b->span = spanFrom(start);
   return b;
}

StmtPtr Parser::parseExprStatement() {
    size_t start = pos_;
    auto first = parseExpression();
    ExprPtr target, value;
    if (match(TokenType::Equals)) {
        if (!first->isLvalue)
            diags_.error(first->span, "left side of '=' is not assignable");
        target = std::move(first);
        value = parseExpression();
    } else {
        value = std::move(first);
    }
    expect(TokenType::SColon, "';'");
    auto s = std::make_unique<ExprStmt>(std::move(target), std::move(value));
    s->span = spanFrom(start);
    return s;
}

// Expressions

ExprPtr Parser::parseExpression() {
    return parseLogicalOr();
}

ExprPtr Parser::parseLogicalOr() {
    size_t start = pos_;
    ExprPtr lhs = parseLogicalAnd();
    while (match(TokenType::OrOr)) {
        auto n = std::make_unique<LogicalExpr>(std::move(lhs), LogicalOp::Or,
                parseLogicalAnd());
        n->span = spanFrom(start);
        lhs = std::move(n);
    }
    return lhs;
}

ExprPtr Parser::parseLogicalAnd() {
    size_t start = pos_;
    ExprPtr lhs = parseComparison();
    while (match(TokenType::AndAnd)) {
        auto n = std::make_unique<LogicalExpr>(std::move(lhs), LogicalOp::And,
                parseComparison());
        n->span = spanFrom(start);
        lhs = std::move(n);
    }
    return lhs;
}

inline BinOp isComparisonOp(TokenType t) {
    switch (t) {
        case TokenType::Equal:
            return BinOp::Eq;
        case TokenType::NEqual:
            return BinOp::Ne;
        case TokenType::LArrow:
            return BinOp::Lt;
        case TokenType::LEqual:
            return BinOp::Le;
        case TokenType::RArrow:
            return BinOp::Gt;
        case TokenType::GEqual:
            return BinOp::Ge;
        default: return BinOp::Add; // Check for this
    }
}

ExprPtr Parser::parseComparison() {
    size_t start = pos_;
    ExprPtr lhs = parseAdditive();
    BinOp op = isComparisonOp(peek().type);
    if (op != BinOp::Add) {
        advance();
        auto n = std::make_unique<BinaryExpr>(std::move(lhs), op, parseAdditive());
        n->span = spanFrom(start);
        lhs = std::move(n);
        BinOp check = isComparisonOp(peek().type);
        if (check != BinOp::Add) {
            fail(peek(), "Cannot chain comparison operators");
        }
    }
    return lhs;
}

ExprPtr Parser::parseAdditive() {
    size_t start = pos_;
    ExprPtr lhs = parseMultiplicative();
    for (;;) {
        BinOp op;
        if (match(TokenType::Plus)) op = BinOp::Add;
        else if (match(TokenType::Minus)) op = BinOp::Sub;
        else return lhs;
        auto n = std::make_unique<BinaryExpr>(std::move(lhs), op, parseMultiplicative());
        n->span = spanFrom(start);
        lhs = std::move(n);
    }
}

ExprPtr Parser::parseMultiplicative() {
    size_t start = pos_;
    ExprPtr lhs = parseCast();
    for (;;) {
        BinOp op;
        if (match(TokenType::Mul)) op = BinOp::Mul;
        else if (match(TokenType::Div)) op = BinOp::Div;
        else return lhs;
        auto n = std::make_unique<BinaryExpr>(std::move(lhs), op, parseCast());
        n->span = spanFrom(start);
        lhs = std::move(n);
    }
}

ExprPtr Parser::parseCast() {
    size_t start = pos_;
    ExprPtr e = parseUnary();
    while (match(TokenType::As)) {
        Type t = parseType();
        auto c = std::make_unique<CastExpr>(std::move(e), t);
        c->span = spanFrom(start);
        e = std::move(c);
    }
    return e;
}

ExprPtr Parser::parseUnary() {
    size_t start = pos_;
    if (check(TokenType::Minus) || check(TokenType::Exclam)) {
        UnaryOp op = check(TokenType::Minus) ? UnaryOp::Neg : UnaryOp::Not;
        advance();
        auto e = std::make_unique<UnaryExpr>(op, parseUnary());
        e->span = spanFrom(start);
        return e;
    }
    return parsePostfix();
}

ExprPtr Parser::parsePostfix() {
    size_t start = pos_;
    ExprPtr e = parsePrimary();
    for (;;) {
        if (match(TokenType::LSquare)) {
            auto idx = parseExpression();
            expect(TokenType::RSquare, "']'");
            auto n = std::make_unique<IndexExpr>(std::move(e), std::move(idx));
            n->span = spanFrom(start);
            e = std::move(n);
        } else if (match(TokenType::LParen)) {
            auto args = parseArgs();
            expect(TokenType::RParen, "')'");
            auto n = std::make_unique<CallExpr>(std::move(e), std::move(args));
            n->span = spanFrom(start);
            e = std::move(n);
        } else {
            return e;
        }
    }
}

std::vector<ExprPtr> Parser::parseArgs() {
    std::vector<ExprPtr> args;
    if (check(TokenType::RParen)) return args;
    do { args.push_back(parseExpression()); } while (match(TokenType::Comma));
    return args;
}

ExprPtr Parser::parsePrimary() {
    size_t start = pos_;
    ExprPtr e;
    switch(peek().type) {
        case TokenType::Number:
            e = std::make_unique<IntLiteral>(advance().intValue);
            break;
        case TokenType::Char:
            e = std::make_unique<IntLiteral>(advance().intValue);
            e->type = Type::u8();
            break;
        case TokenType::String:
            e = std::make_unique<StringLiteral>(advance().text);
            break;
        case TokenType::True:
            advance();
            e = std::make_unique<BoolLiteral>(true);
            break;
        case TokenType::False:
            advance();
            e = std::make_unique<BoolLiteral>(false);
            break;
        case TokenType::Identifier:
            e = std::make_unique<NameExpr>(strings_.addString(advance().text));
            break;
        case TokenType::LParen: {
            advance();
            e = parseExpression();
            expect(TokenType::RParen, "')'");
            return e;
        }
        default:
            fail(peek(), "expected an expression, found " + tokenName(peek().type));

    }
    e->span = spanFrom(start);
    return e;
}

// Fix after error, to catch more errors

void Parser::fix() {
    fix_++;
    if (fix_ > 20) {
        diags_.listErrors();
        exit(EXIT_FAILURE);
    }
    while (!atEnd()) {
        if (previous().type == TokenType::SColon) return;
        switch(peek().type) {
            case TokenType::Fn: case TokenType::Let:
            case TokenType::Mut: case TokenType::Ret:
            case TokenType::Break: case TokenType:: Continue:
            case TokenType::If: case TokenType::While:
            return;
            default: advance();
        }
    }
}



// Types

Type Parser::parseType() {
    switch(peek().type) {
        case TokenType::Void: advance(); return Type::voidType();
        case TokenType::I64: advance(); return Type::i64();
        case TokenType::U64: advance(); return Type::u64();
        case TokenType::I32: advance(); return Type::i32();
        case TokenType::U32: advance(); return Type::u32();
        case TokenType::I16: advance(); return Type::i16();
        case TokenType::U16: advance(); return Type::u16();
        case TokenType::I8: advance(); return Type::i8();
        case TokenType::U8: advance(); return Type::u8();
        case TokenType::Bool: advance(); return Type::boolType();
        case TokenType::LSquare: {
            advance();
            Type inner = parseType();
            expect(TokenType::RSquare, "']' closing array type");
            return Type::array(inner);
        }
        default: fail(peek(), "expected a type, found " + tokenName(peek().type));
        return Type::unresolved();
    }
}

// Parser helper

const Token& Parser::peek(int n) const {
    size_t i = pos_ + static_cast<size_t>(n);
    return i < toks_.size() ? toks_[i] : toks_.back(); //    
}

const Token& Parser::previous() const { return toks_[pos_ > 0 ? pos_ - 1 : 0]; }
bool Parser::atEnd() const { return peek().type == TokenType::End; }
bool Parser::check(TokenType t) const { return peek().type == t; }

const Token& Parser::advance() {
    if (!atEnd()) pos_++;
    return previous();
}

bool Parser::match(TokenType t) {
    if (!check(t)) return false;
    advance();
    return true;
}

const Token& Parser::expect(TokenType t, const char* what) {
    if (check(t)) return advance();
    fail(peek(), std::string("expected ") + what + ", found " + tokenName(peek().type));
    return advance();
}

void Parser::fail(const Token& at, const std::string& msg) {
    diags_.error({at.line, at.col, at.length}, msg);
    throw 1;
}

Span Parser::spanFrom(size_t startIdx) const {
    const Token& s = toks_[startIdx];
    const Token& e = toks_[pos_ > 0 ? pos_ - 1 : 0];
    unsigned int length = (e.line == s.line && e.col >= s.col)
        ? (e.col - s.col) + e.length : s.length;
    return { s.line, s.col, length };
}
