#pragma once

#include "ast.h"
#include "stringtable.h"
#include <unordered_map>

class TypeCheck : public Visitor {
    public:
    TypeCheck(StringTable table) : table_(table) {}
    Type visit(ProgramNode&) override;
    Type visit(FunctionNode&) override;
    Type visit(OperandNode&) override;

    Type visit(LetStatementNode&) override;
    Type visit(BlockStatementNode&) override;
    Type visit(ReassignStatementNode&) override;
    Type visit(ReturnStatementNode&) override;
    Type visit(ExpressionStatementNode&) override;

    Type visit(BinaryExpressionNode&) override;
    Type visit(TermExpressionNode&) override;
    Type visit(CallExpressionNode&) override;

    unsigned int errors;
    private:
    StringTable table_;
    std::unordered_map<int, Type> varType;
    std::unordered_map<int, Type> funcType;
    std::unordered_map<int, std::vector<Type>> funcOpCount;
};
