#include "parser.h"
#include "ast.h"
#include "token.h"

#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <utility>

Parser::Parser(std::vector<Token> tokens, StringTable& table) : tokens_(std::move(tokens)), pos_(0),  table_(table){}


ProgramNode Parser::parse() {
    ProgramNode program;
    /*std::unique_ptr<ReturnStatementNode> stmt = std::make_unique<ReturnStatementNode>(std::make_unique<TermExpressionNode>(1, false, Type::i32));
        
    std::unique_ptr<OperandNode> op = std::make_unique<OperandNode>(table_.addString("input"), Type::String);
    std::unique_ptr<OperandNode> op2 = std::make_unique<OperandNode>(table_.addString("length"), Type::i32);
    std::vector<std::unique_ptr<OperandNode>> operands;
    operands.push_back(std::move(op));
    operands.push_back(std::move(op2));
    std::unique_ptr<FunctionNode> printFunction = std::make_unique<FunctionNode>(table_.addString("print"), Type::i32, std::move(stmt), std::move(operands));
    program.functions.push_back(std::move(printFunction));*/
    while (!check(TokenType::End)) {
        program.functions.push_back(function());
    }
    
    return program;
}

std::unique_ptr<FunctionNode> Parser::function() {
    const Token& name = expect(TokenType::Identifier, "function identifier");
    expect(TokenType::LParen, "(");
    std::vector<std::unique_ptr<OperandNode>> operands;
    if (!check(TokenType::RParen)) {
        operands.push_back(operand());
        while (match(TokenType::Comma))
            operands.push_back(operand());
    }
    expect(TokenType::RParen, ")");
    expect(TokenType::Colon, ":");
    Type t = expectType();
    expect(TokenType::Equals, "=>");
    expect(TokenType::RArrow, "=>");
    std::unique_ptr<StatementNode> functionStatement = statement();
    return std::make_unique<FunctionNode>(name.value, t, std::move(functionStatement), std::move(operands));
}

std::unique_ptr<OperandNode> Parser::operand() {
    Token identifier = expect(TokenType::Identifier, "identifier");
    expect(TokenType::Colon, ": in operands list");
    Type type = expectType();
    return std::make_unique<OperandNode>(identifier.value, type);
}

std::unique_ptr<StatementNode> Parser::statement() {
    if (check(TokenType::LCurly)) {
        return blockStatement();
    }
    if (check(TokenType::Let) || check(TokenType::Const)) {
        return letStatement();
    }
    if (check(TokenType::Ret)) {
        return returnStatement();
    }
    if (check(TokenType::If)) {
        return ifStatement();
    }
    std::unique_ptr<ExpressionNode> e = expression();
    if (check(TokenType::Equals)) {
        TermExpressionNode* raw = dynamic_cast<TermExpressionNode*>(e.get());
        bool isLvalue = raw && !dynamic_cast<CallExpressionNode*>(e.get());
        if (!isLvalue) {
            std::cerr << "Parse Error: invalid assignment target!\n";
            exit(EXIT_FAILURE);
        }
        expect(TokenType::Equals, "=");
        std::unique_ptr<ExpressionNode> rhs = expression();
        expect(TokenType::SColon, ";");
        e.release();
        return std::make_unique<ReassignStatementNode>(std::unique_ptr<TermExpressionNode>(raw), std::move(rhs));
    }
    expect(TokenType::SColon, ";");
    return std::make_unique<ExpressionStatementNode>(std::move(e)); // worse case, just expect an expression
}

std::unique_ptr<LetStatementNode> Parser::letStatement() {
    bool isConst = false;
    if (check(TokenType::Const)) {
        isConst = true;
        expect(TokenType::Const, "const or let");
    } else {
        expect(TokenType::Let, "const or let");
    }
    const Token& name = expect(TokenType::Identifier, "identifier");
    expect(TokenType::Colon, ":");
    Type t = expectType();
    expect(TokenType::Equals, "=");
    std::unique_ptr<ExpressionNode> init = expression();
    expect(TokenType::SColon, ";");
    return std::make_unique<LetStatementNode>(name.value, t, std::move(init), isConst);
}

std::unique_ptr<BlockStatementNode> Parser::blockStatement() {
    expect(TokenType::LCurly, "{");
    std::vector<std::unique_ptr<StatementNode>> statements;
    while (!check(TokenType::RCurly) && !check(TokenType::End)) {
        statements.push_back(statement());
    }
    expect(TokenType::RCurly, "}");
    return std::make_unique<BlockStatementNode>(std::move(statements));
}

std::unique_ptr<ReassignStatementNode> Parser::reassignStatement() {
    std::unique_ptr<TermExpressionNode> target = termExpression();
    expect(TokenType::Equals, "=");
    std::unique_ptr<ExpressionNode> e = expression();
    expect(TokenType::SColon, ";");
    return std::make_unique<ReassignStatementNode>(std::move(target), std::move(e));
}

std::unique_ptr<ReturnStatementNode> Parser::returnStatement() {
    expect(TokenType::Ret, "ret");
    std::unique_ptr<ExpressionNode> e = expression();
    expect(TokenType::SColon, ";");
    return std::make_unique<ReturnStatementNode>(std::move(e));
}

std::unique_ptr<IfStatementNode> Parser::ifStatement() {
    expect(TokenType::If, "if");
    std::unique_ptr<ExpressionNode> expr = expression();
    std::unique_ptr<BlockStatementNode> ifBlock = blockStatement();
    if (check(TokenType::Else)) {
        expect(TokenType::Else, "else");
        if (check(TokenType::LCurly)) {
            std::unique_ptr<StatementNode> elseBlock = statement();
            return std::make_unique<IfStatementNode>(std::move(expr), std::move(ifBlock), std::move(elseBlock), true);
        } else {
            std::unique_ptr<StatementNode> elseBlock = ifStatement();
            return std::make_unique<IfStatementNode>(std::move(expr), std::move(ifBlock), std::move(elseBlock), true);
        }
    }
    return std::make_unique<IfStatementNode>(std::move(expr), std::move(ifBlock), nullptr, false);
}

std::unique_ptr<ExpressionNode> Parser::expression() {
    return comparisonExpression();
}

std::unique_ptr<ExpressionNode> Parser::comparisonExpression() {
    std::unique_ptr<ExpressionNode> lhs = additiveExpression();
    if (check(TokenType::Equal)) {
        expect(TokenType::Equal, "==");
        expect(TokenType::Equals, "==");
        std::cout << "EQuals detected with next token: " << (int)peek(0).type << "\n";
        return std::make_unique<BinaryExpressionNode>(std::move(lhs), BinOp::Eq, additiveExpression());
    }

    return lhs;
}

std::unique_ptr<ExpressionNode> Parser::additiveExpression() {
    std::unique_ptr<ExpressionNode> lhs = termExpression();
    while (check(TokenType::Plus) || check(TokenType::Minus)) {
        BinOp op = check(TokenType::Plus) ? BinOp::Add : BinOp::Sub;
        advance();
        lhs = std::make_unique<BinaryExpressionNode>(std::move(lhs), op, termExpression());
    }
    return lhs;
}

std::unique_ptr<TermExpressionNode> Parser::termExpression() {
    std::unique_ptr<TermExpressionNode> e = primaryExpression();
    while (check(TokenType::LSquare)) {
        expect(TokenType::LSquare, "[");
        std::unique_ptr<ExpressionNode> index = expression();
        expect(TokenType::RSquare, "]");
        e = std::make_unique<IndexExpressionNode>(std::move(e), std::move(index));
    }
    return e;
}

std::unique_ptr<TermExpressionNode> Parser::primaryExpression() {
    if (check(TokenType::Number)) {
        Token tok = expect(TokenType::Number, "number or identifier");
        return std::make_unique<TermExpressionNode>(tok.value, false, Type::u8(), true);
    }
    if (check(TokenType::Identifier)) {
        Token tok = expect(TokenType::Identifier, "number or identifier");
        if (check(TokenType::LParen)) {
            return callExpression(tok);
        }
        return std::make_unique<TermExpressionNode>(tok.value, true, Type::Void(), true);
    }
    if (check(TokenType::Speech)) {
        expect(TokenType::Speech, "\"");
        Token str = expect(TokenType::String, "string literal");
        std::unique_ptr<TermExpressionNode> expr = std::make_unique<TermExpressionNode>(str.value, false, Type::array(Type::u8()), true);
        expr->type = Type::array(Type::u8());
        expect(TokenType::Speech, "\"");
        return expr;
    }
    if (check(TokenType::True)) {
        Token tok = expect(TokenType::True, "true");
        return std::make_unique<TermExpressionNode>(tok.value, false, Type::Bool(), true);
    }
    if (check(TokenType::False)) {
        Token tok = expect(TokenType::False, "false");
        return std::make_unique<TermExpressionNode>(tok.value, false, Type::Bool(), true);
    }
    if (checkType().type != TypeKind::Void) {
        return castExpression();
    }
    std::cerr << "Parse Error: expected number or identifier at line: " << peek().line << "!\n";
    exit(EXIT_FAILURE);
}

std::unique_ptr<CallExpressionNode> Parser::callExpression(const Token& tok) {
    expect(TokenType::LParen, "(");
    std::vector<std::unique_ptr<ExpressionNode>> operands;
    if (!check(TokenType::RParen)) {
        operands.push_back(expression());
        while (match(TokenType::Comma))
            operands.push_back(expression());
    }
    expect(TokenType::RParen, ")");
    return std::make_unique<CallExpressionNode>(tok.value, true, std::move(operands), Type::Void());
}

std::unique_ptr<CastExpressionNode> Parser::castExpression() {
    Type t = expectType();
    expect(TokenType::LArrow, "<");
    expect(TokenType::Minus, "-");
    std::unique_ptr<ExpressionNode> expr = comparisonExpression();
    return std::make_unique<CastExpressionNode>(std::move(expr), t);
}

// Helpers

const Token& Parser::peek() const {
    return tokens_.at(pos_);
}

const Token& Parser::peek(int idx) const {
    return tokens_.at(pos_ + idx);
}

const Token& Parser::advance() {
    return tokens_.at(pos_++);
}

bool Parser::check(TokenType t)  const {
    return peek().type == t;
}

bool Parser::check(TokenType t, int idx)  const {
    return peek(idx).type == t;
}

bool Parser::match(TokenType t) {
    if (check(t)) {
        this->pos_++;
        return true;
    }
    return false;
}

const Token& Parser::expect(TokenType type, const char* what) {
    if (check(type)) return advance();
    Token t = peek();
    std::cerr << "Parse Error: expected " << what << "of type: " << (int)type << " at token: " << (int)t.type << " at line: " << t.line << std::endl;
    exit(EXIT_FAILURE);
}

Type Parser::checkType() {
    if (check(TokenType::LSquare)) {
        return Type::array(Type::Void());
    }
    if (check(TokenType::i32)) return Type::i32();
    if (check(TokenType::Bool)) return Type::Bool();
    if (check(TokenType::u8)) return Type::u8();
    return Type::Void();
}

Type Parser::expectType() {
    if (check(TokenType::LSquare)) {
        expect(TokenType::LSquare, "[");
        Type t = expectType();
        expect(TokenType::RSquare, "]");
        return Type::array(t);
    }
    if (match(TokenType::i32)) return Type::i32();
    if (match(TokenType::Bool)) return Type::Bool();
    if (match(TokenType::u8)) return Type::u8();
    std::cerr << "Parse Error: Expected type at line " << peek().line << "!\n";
    exit(EXIT_FAILURE);
}
