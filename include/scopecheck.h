#pragma once

#include "ast.h"
#include "stringtable.h"


class ScopeCheck : Visitor {
    public:
    ScopeCheck(StringTable& table) : table_(table) {}
    void visit(ProgramNode&) override;
    void visit(FunctionNode&) override;
    void visit(OperandNode&) override;

    void visit(LetStatementNode&) override;
    void visit(BlockStatementNode&) override;
    void visit(ReassignStatementNode&) override;
    void visit(ReturnStatementNode&) override;
    void visit(ExpressionStatementNode&) override;

    void visit(BinaryExpressionNode&) override;
    void visit(TermExpressionNode&) override;
    void visit(CallExpressionNode&) override;
    private:
    std::vector<std::vector<int>> scopes_;
    unsigned int initCounter_;
    StringTable table_;

    bool existsInCurrentScope(int idx);
    bool existsAtAll(int idx);
};
