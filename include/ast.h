#pragma once

#include <memory>
#include <vector>
struct ProgramNode;
struct FunctionNode;
struct OperandNode;

struct StatementNode;
struct LetStatementNode;
struct BlockStatementNode;
struct ReassignStatementNode;
struct ReturnStatementNode;
struct ExpressionStatementNode;

struct ExpressionNode;
struct BinaryExpressionNode;
struct TermExpressionNode;
struct CallExpressionNode;

enum class Type { Unknown, i32, String };

class Visitor {
    public:
    virtual ~Visitor() = default;
    virtual void visit(ProgramNode&) = 0;
    virtual void visit(FunctionNode&) = 0;
    virtual void visit(OperandNode&) = 0;

    virtual void visit(LetStatementNode&) = 0;
    virtual void visit(BlockStatementNode&) = 0;
    virtual void visit(ReassignStatementNode&) = 0;
    virtual void visit(ReturnStatementNode&) = 0;
    virtual void visit(ExpressionStatementNode&) = 0;

    virtual void visit(TermExpressionNode&) = 0;
    virtual void visit(BinaryExpressionNode&) = 0;
    virtual void visit(CallExpressionNode&) = 0;
};

struct StatementNode{
    virtual ~StatementNode() = default;
    virtual void accept(Visitor&) = 0;
}; 

struct LetStatementNode : StatementNode {
    int idx; // string table idx for name of ident
    Type type;
    std::unique_ptr<ExpressionNode> expr;
    LetStatementNode(int idx, Type type, std::unique_ptr<ExpressionNode> expr) : idx(idx), type(type), expr(std::move(expr)) {}
    void accept(Visitor& v) override { v.visit(*this); }
};

struct BlockStatementNode : StatementNode {
    std::vector<std::unique_ptr<StatementNode>> statements;
    BlockStatementNode(std::vector<std::unique_ptr<StatementNode>> statements) : statements(std::move(statements)) { }
    void accept(Visitor& v) override { v.visit(*this); }
};

struct ReassignStatementNode : StatementNode {
    int idx;
    std::unique_ptr<ExpressionNode> expr;
    ReassignStatementNode(int idx, std::unique_ptr<ExpressionNode> expr) : idx(idx), expr(std::move(expr)) {  }
    void accept(Visitor& v) override { v.visit(*this); }
};

struct ReturnStatementNode : StatementNode {
    std::unique_ptr<ExpressionNode> expr;
    ReturnStatementNode(std::unique_ptr<ExpressionNode> expr) : expr(std::move(expr)) {}
    void accept(Visitor& v) override { v.visit(*this); }
};

struct ExpressionStatementNode : StatementNode {
    std::unique_ptr<ExpressionNode> expr;

    ExpressionStatementNode(std::unique_ptr<ExpressionNode> expr) : expr(std::move(expr)) { }
    void accept(Visitor& v) override { v.visit(*this); }
};

struct ExpressionNode {
    Type inferredType = Type::Unknown;
    virtual ~ExpressionNode() = default;
    virtual void accept(Visitor&) = 0;
};

struct TermExpressionNode : ExpressionNode {
    int value;
    Type type;
    bool isIdent;
    explicit TermExpressionNode(int v, bool isIdent) : value(v), isIdent(isIdent) {}
    void accept(Visitor& v) override { v.visit(*this); }
};

struct BinaryExpressionNode : ExpressionNode {
    std::unique_ptr<ExpressionNode> l;
    std::unique_ptr<ExpressionNode> r;
    char op;
    BinaryExpressionNode(std::unique_ptr<ExpressionNode> l, char op, std::unique_ptr<ExpressionNode> r) : l(std::move(l)), op(op), r(std::move(r)) { }
    void accept(Visitor& v) override { v.visit(*this); }
};

struct CallExpressionNode : TermExpressionNode {
    std::vector<std::unique_ptr<TermExpressionNode>> operands;
    CallExpressionNode(int v, bool isIdent, std::vector<std::unique_ptr<TermExpressionNode>> operands) : TermExpressionNode(v, isIdent), operands(std::move(operands)) {}
    void accept(Visitor& v) override { v.visit(*this); }
};

struct OperandNode {
    int identIdx;
    Type type;
    
    OperandNode(int identIdx, Type type) : identIdx(identIdx), type(type) { }
    void accept(Visitor& v) {
        v.visit(*this);
    }
};

struct FunctionNode {
    int nameIdx;
    unsigned int localCount;
    Type type;
    std::unique_ptr<StatementNode> statement;
    std::vector<std::unique_ptr<OperandNode>> operands;
    FunctionNode(int nameIdx, Type returnType, std::unique_ptr<StatementNode> statement, std::vector<std::unique_ptr<OperandNode>> operands) : nameIdx(nameIdx), type(returnType), statement(std::move(statement)), operands(std::move(operands)) {}
    void accept(Visitor& v) {
        v.visit(*this);
    }
};

struct ProgramNode {
    std::vector<std::unique_ptr<FunctionNode>> functions;
    void accept(Visitor& v) {
        v.visit(*this);
    }
};
