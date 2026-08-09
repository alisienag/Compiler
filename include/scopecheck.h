#pragma once

#include "ast.h"
#include "stringtable.h"


class ScopeCheck : Visitor {
    public:
    ScopeCheck(StringTable& table) : table_(table) {}
    Type visit(ProgramNode&) override;
    Type visit(FunctionNode&) override;
    Type visit(OperandNode&) override;

    Type visit(LetStatementNode&) override;
    Type visit(BlockStatementNode&) override;
    Type visit(ReassignStatementNode&) override;
    Type visit(ReturnStatementNode&) override;
    Type visit(ExpressionStatementNode&) override;
    Type visit(IfStatementNode&) override;

    Type visit(BinaryExpressionNode&) override;
    Type visit(TermExpressionNode&) override;
    Type visit(CallExpressionNode&) override;
    
    unsigned int errors;
    private:
    std::vector<std::vector<int>> scopes_;
    unsigned int initCounter_;
    StringTable& table_;

    bool existsInCurrentScope(int idx);
    bool existsAtAll(int idx);
};
