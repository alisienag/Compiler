#pragma once

#include <vector>
#include <string>

#include "token.h"
#include "stringtable.h"
#include "diagnostics.h"
#include "ast.h"

class Parser {
    public:
        Parser(std::vector<Token> tokens, StringTable& strings, Diagnostics& diags)
            : toks_(std::move(tokens)), strings_(strings), diags_(diags) {}
        std::unique_ptr<Program> parseProgram();
        void listErrors() const;
    private:
        const Token& peek(int n = 0) const;
        const Token& previous() const;
        bool atEnd() const;
        bool check(TokenType t) const;
        bool match(TokenType t);
        const Token& advance();
        const Token& expect(TokenType t, const char* what);

        void fail(const Token& at, const std::string& msg);
        void fix();
        
        Span spanFrom(size_t startIdx) const;
        
        std::unique_ptr<FunctionDecl> parseFunction();
        std::unique_ptr<Param> parseParam();

        // Statements

        StmtPtr parseStatement();
        StmtPtr parseLet();
        StmtPtr parseReturn();
        StmtPtr parseIf();
        StmtPtr parseWhile();
        std::unique_ptr<BlockStmt> parseBlock();
        StmtPtr parseExprStatement();

        // Expressions
        ExprPtr parseExpression();
        ExprPtr parseLogicalOr();
        ExprPtr parseLogicalAnd();
        ExprPtr parseComparison();
        ExprPtr parseAdditive();
        ExprPtr parseMultiplicative();
        ExprPtr parseCast();
        ExprPtr parseUnary();
        ExprPtr parsePostfix();
        std::vector<ExprPtr> parseArgs();
        ExprPtr parsePrimary();

        // Types
        bool startsType(TokenType t) const;
        Type parseType();

        std::vector<Token> toks_;
        StringTable& strings_;
        Diagnostics& diags_;
        size_t pos_ = 0;
        unsigned int fix_ = 0;
};
