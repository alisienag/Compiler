#include "parser.h"
#include "ast.h"

#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <utility>

Parser::Parser(std::vector<Token> tokens, StringTable table) : tokens_(std::move(tokens)), pos_(0),  table_(table){}


ProgramNode Parser::parse() {
    ProgramNode program;
    std::unique_ptr<ExpressionStatementNode> stmt = std::make_unique<ExpressionStatementNode>(std::make_unique<TermExpressionNode>(1, false, Type::i32));
        
    std::unique_ptr<OperandNode> op = std::make_unique<OperandNode>(table_.addString("input"), Type::String);
    std::unique_ptr<OperandNode> op2 = std::make_unique<OperandNode>(table_.addString("length"), Type::i32);
    std::vector<std::unique_ptr<OperandNode>> operands;
    operands.push_back(std::move(op));
    operands.push_back(std::move(op2));
    std::unique_ptr<FunctionNode> printFunction = std::make_unique<FunctionNode>(table_.addString("print"), Type::i32, std::move(stmt), std::move(operands));
    program.functions.push_back(std::move(printFunction));
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
    if (check(TokenType::Let)) {
        return letStatement();
    }
    if (check(TokenType::Identifier) && check(TokenType::Equals, 1)) {
        return reassignStatement();
    }
    if (check(TokenType::Ret)) {
        return returnStatement();
    }
    std::unique_ptr<ExpressionNode> e = expression();
    expect(TokenType::SColon, ";");
    return std::make_unique<ExpressionStatementNode>(std::move(e)); // worse case, just expect an expression
}

std::unique_ptr<LetStatementNode> Parser::letStatement() {
    expect(TokenType::Let, "let");
    const Token& name = expect(TokenType::Identifier, "identifier");
    expect(TokenType::Colon, ":");
    Type t = expectType();
    expect(TokenType::Equals, "=");
    std::unique_ptr<ExpressionNode> init = expression();
    expect(TokenType::SColon, ";");
    return std::make_unique<LetStatementNode>(name.value, t, std::move(init));
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
    const Token& tok = expect(TokenType::Identifier, "identifier");
    expect(TokenType::Equals, "=");
    std::unique_ptr<ExpressionNode> e = expression();
    expect(TokenType::SColon, ";");
    return std::make_unique<ReassignStatementNode>(tok.value, std::move(e));
}

std::unique_ptr<ReturnStatementNode> Parser::returnStatement() {
    expect(TokenType::Ret, "ret");
    std::unique_ptr<ExpressionNode> e = expression();
    expect(TokenType::SColon, ";");
    return std::make_unique<ReturnStatementNode>(std::move(e));
}

std::unique_ptr<ExpressionNode> Parser::expression() {
    std::unique_ptr<ExpressionNode> lhs = termExpression();
    while (check(TokenType::Plus) || check(TokenType::Minus)) {
        char op = check(TokenType::Plus) ? '+' : '-';
        advance();
        lhs = std::make_unique<BinaryExpressionNode>(std::move(lhs), op, termExpression());
    }
    return lhs;
}

std::unique_ptr<TermExpressionNode> Parser::termExpression() {
    if (check(TokenType::Number)) {
        Token tok = expect(TokenType::Number, "number or identifier");
        return std::make_unique<TermExpressionNode>(tok.value, false, Type::i32);
    }
    if (check(TokenType::Identifier)) {
        Token tok = expect(TokenType::Identifier, "number or identifier");
        if (check(TokenType::LParen)) {
            return callExpression(tok);
        }
        return std::make_unique<TermExpressionNode>(tok.value, true, Type::Unknown);
    }
    if (check(TokenType::Speech)) {
        expect(TokenType::Speech, "\"");
        Token str = expect(TokenType::String, "string literal");
        std::unique_ptr<TermExpressionNode> expr = std::make_unique<TermExpressionNode>(str.value, false, Type::String);
        expr->type = Type::String;
        expect(TokenType::Speech, "\"");
        return expr;
    }
    std::cerr << "Parse Error: expected number or identifier at line: " << peek().line << "!\n";
    exit(EXIT_FAILURE);
}

std::unique_ptr<CallExpressionNode> Parser::callExpression(const Token& tok) {
    expect(TokenType::LParen, "(");
    std::vector<std::unique_ptr<TermExpressionNode>> operands;
    if (!check(TokenType::RParen)) {
        operands.push_back(termExpression());
        while (match(TokenType::Comma))
            operands.push_back(termExpression());
    }
    expect(TokenType::RParen, ")");
    return std::make_unique<CallExpressionNode>(tok.value, true, std::move(operands), Type::Unknown);
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
    std::cerr << "Parse Error: expected " << what << " at token: " << (int)t.type << " at line: " << t.line << std::endl;
    exit(EXIT_FAILURE);
}

Type Parser::expectType() {
    if (match(TokenType::i32)) return Type::i32;
    if (match(TokenType::String)) return Type::String;
    std::cerr << "Parse Error: Expected type at line " << peek().line << "!\n";
    exit(EXIT_FAILURE);
}
