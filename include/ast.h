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
struct IfStatementNode;

struct ExpressionNode;
struct BinaryExpressionNode;
struct TermExpressionNode;
struct CallExpressionNode;

enum class Type { Unknown, i32, String, Bool };

enum class BinOp { Add, Sub, Mul, Div, Eq, Ne, Lt, Gt, Le, Ge };

class Visitor {
    public:
    virtual ~Visitor() = default;
    virtual Type visit(ProgramNode&) = 0;
    virtual Type visit(FunctionNode&) = 0;
    virtual Type visit(OperandNode&) = 0;

    virtual Type visit(LetStatementNode&) = 0;
    virtual Type visit(BlockStatementNode&) = 0;
    virtual Type visit(ReassignStatementNode&) = 0;
    virtual Type visit(ReturnStatementNode&) = 0;
    virtual Type visit(ExpressionStatementNode&) = 0;
    virtual Type visit(IfStatementNode&) = 0;

    virtual Type visit(TermExpressionNode&) = 0;
    virtual Type visit(BinaryExpressionNode&) = 0;
    virtual Type visit(CallExpressionNode&) = 0;

    static const char* typeName(Type t) {
        switch (t) {
            case Type::i32:     return "i32";
            case Type::Unknown: return "unknown";
            case Type::String:  return "string";
            case Type::Bool: return "bool";
        }
        return "?";
    }
    static const char* opName(BinOp t) {
        switch (t) {
            case BinOp::Add: return "+";
            case BinOp::Sub: return "-";
            case BinOp::Mul: return "*";
            case BinOp::Div: return "/";
            case BinOp::Eq: return "==";
            case BinOp::Ne: return "!=";
            case BinOp::Lt: return "<";
            case BinOp::Gt: return ">";
            case BinOp::Le: return "<=";
            case BinOp::Ge: return ">=";
        }
        return "?";
    }
};

struct StatementNode{
    virtual ~StatementNode() = default;
    virtual Type accept(Visitor&) = 0;
}; 

struct LetStatementNode : StatementNode {
    int idx; // string table idx for name of ident
    Type type;
    std::unique_ptr<ExpressionNode> expr;
    LetStatementNode(int idx, Type type, std::unique_ptr<ExpressionNode> expr) : idx(idx), type(type), expr(std::move(expr)) {}
    Type accept(Visitor& v) override { return v.visit(*this); }
};

struct BlockStatementNode : StatementNode {
    std::vector<std::unique_ptr<StatementNode>> statements;
    BlockStatementNode(std::vector<std::unique_ptr<StatementNode>> statements) : statements(std::move(statements)) { }
    Type accept(Visitor& v) override { return v.visit(*this); }
};

struct ReassignStatementNode : StatementNode {
    int idx;
    std::unique_ptr<ExpressionNode> expr;
    ReassignStatementNode(int idx, std::unique_ptr<ExpressionNode> expr) : idx(idx), expr(std::move(expr)) {  }
    Type accept(Visitor& v) override { return v.visit(*this); }
};

struct ReturnStatementNode : StatementNode {
    std::unique_ptr<ExpressionNode> expr;
    ReturnStatementNode(std::unique_ptr<ExpressionNode> expr) : expr(std::move(expr)) {}
    Type accept(Visitor& v) override { return v.visit(*this); }
};

struct ExpressionStatementNode : StatementNode {
    std::unique_ptr<ExpressionNode> expr;

    ExpressionStatementNode(std::unique_ptr<ExpressionNode> expr) : expr(std::move(expr)) { }
    Type accept(Visitor& v) override { return v.visit(*this); }
};

struct IfStatementNode : StatementNode {
    std::unique_ptr<ExpressionNode> cond;
    std::unique_ptr<BlockStatementNode> ifNode;
    std::unique_ptr<StatementNode> elseNode;
    bool hasElse;
    IfStatementNode(std::unique_ptr<ExpressionNode> cond, std::unique_ptr<BlockStatementNode> ifNode, std::unique_ptr<StatementNode> elseNode, bool hasElse) : cond(std::move(cond)), ifNode(std::move(ifNode)), elseNode(std::move(elseNode)), hasElse(hasElse) {}
    Type accept(Visitor& v) override { return v.visit(*this); }
};

struct ExpressionNode {
    Type inferredType = Type::Unknown;
    ExpressionNode(Type t) : inferredType(t) {}
    virtual ~ExpressionNode() = default;
    virtual Type accept(Visitor&) = 0;
};

struct TermExpressionNode : ExpressionNode {
    int value;
    Type type;
    bool isIdent;
    explicit TermExpressionNode(int v, bool isIdent, Type t) : ExpressionNode(t), value(v), type(t), isIdent(isIdent) {}
    Type accept(Visitor& v) override { return v.visit(*this); }
};

struct BinaryExpressionNode : ExpressionNode {
    std::unique_ptr<ExpressionNode> l;
    std::unique_ptr<ExpressionNode> r;
    BinOp op;
    BinaryExpressionNode(std::unique_ptr<ExpressionNode> l, BinOp op, std::unique_ptr<ExpressionNode> r) : ExpressionNode(Type::Unknown), l(std::move(l)), op(op), r(std::move(r)) { }
    Type accept(Visitor& v) override { return v.visit(*this); }
};

struct CallExpressionNode : TermExpressionNode {
    std::vector<std::unique_ptr<ExpressionNode>> operands;
    CallExpressionNode(int v, bool isIdent, std::vector<std::unique_ptr<ExpressionNode>> operands, Type t) : TermExpressionNode(v, isIdent, t), operands(std::move(operands)) {}
    Type accept(Visitor& v) override { return v.visit(*this); }
};

struct OperandNode {
    int identIdx;
    Type type;
    
    OperandNode(int identIdx, Type type) : identIdx(identIdx), type(type) { }
    Type accept(Visitor& v) {
        return v.visit(*this);
    }
};

struct FunctionNode {
    int nameIdx;
    unsigned int localCount;
    Type type;
    std::unique_ptr<StatementNode> statement;
    std::vector<std::unique_ptr<OperandNode>> operands;
    FunctionNode(int nameIdx, Type returnType, std::unique_ptr<StatementNode> statement, std::vector<std::unique_ptr<OperandNode>> operands) : nameIdx(nameIdx), type(returnType), statement(std::move(statement)), operands(std::move(operands)) {}
    Type accept(Visitor& v) {
        return v.visit(*this);
    }
};

struct ProgramNode {
    std::vector<std::unique_ptr<FunctionNode>> functions;
    Type accept(Visitor& v) {
        return v.visit(*this);
    }
};
