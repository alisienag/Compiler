#pragma once

#include "ast.h"
#include "stringtable.h"
#include <algorithm>


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
    Type visit(CastExpressionNode&) override;
    Type visit(IndexExpressionNode&) override;
    
    unsigned int errors;
    private:
    unsigned int initCounter_;
    StringTable& table_;
    std::vector<std::vector<int>> scopes_;
    std::vector<std::vector<int>> const_;
    void enterScope() {
        scopes_.push_back(std::vector<int>());
        const_.push_back(std::vector<int>());
    }
    void exitScope() {
        scopes_.pop_back();
        const_.pop_back();
    }
    void pushVariable(int idx, bool isConst) {
        scopes_.back().push_back(idx);
        if (isConst)
            const_.back().push_back(idx);
    }

    bool existsInCurrentScope(int idx) {
        if (std::find(scopes_.back().begin(), scopes_.back().end(), idx) != scopes_.back().end()) {
            return true;
        } else {
            return false;
        }

    }

    int existsAtAll(const std::vector<std::vector<int>>& vec, int idx) {
        if (vec.size() == 0) { return 0; }
        for (std::size_t i = vec.size()-1; i > 0; i--) {
            auto& v = vec.at(i);
            if (std::find(v.begin(), v.end(), idx) != v.end()) {
                return i;
            }
        }
        return 0;
    }


    bool isConst;
};
