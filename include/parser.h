#pragma once
#include "ast.h"
#include "stringtable.h"
#include "token.h"
#include <vector>

class Parser {
    public:
        explicit Parser(std::vector<Token> tokens, StringTable table);
        ProgramNode parse();
    private:
        std::vector<Token> tokens_;
        std::size_t pos_;
        StringTable table_;
        const Token& peek() const;
        const Token& peek(int idx) const;
        const Token& advance();

        bool check(TokenType t) const;
        bool check(TokenType t, int idx) const;
        bool match(TokenType t);
        
        const Token& expect(TokenType type, const char* what);

        Type expectType();
        
        std::unique_ptr<FunctionNode> function();
        std::unique_ptr<OperandNode> operand();

        std::unique_ptr<StatementNode> statement();
        std::unique_ptr<LetStatementNode> letStatement();
        std::unique_ptr<BlockStatementNode> blockStatement();
        std::unique_ptr<ReassignStatementNode> reassignStatement();
        std::unique_ptr<ReturnStatementNode> returnStatement();

        std::unique_ptr<ExpressionNode> expression();
        std::unique_ptr<TermExpressionNode> termExpression();
        std::unique_ptr<BinaryExpressionNode> binaryExpression(std::unique_ptr<TermExpressionNode> l);
        std::unique_ptr<CallExpressionNode> callExpression(const Token& tok);
};
