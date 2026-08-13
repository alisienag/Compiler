#pragma once

#include <memory>
#include <vector>
#include <string>
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
struct CastExpressionNode;
struct IndexExpressionNode;

enum class TypeKind { Void, i64, i32, u8, Bool, Array };

struct Type {
    TypeKind type;
    std::shared_ptr<Type> element;
    static Type i64() { return { TypeKind::i64, nullptr}; }
    static Type i32() { return { TypeKind::i32, nullptr}; }
    static Type u8() { return { TypeKind::u8, nullptr }; }
    static Type Bool() { return { TypeKind::Bool, nullptr}; }
    static Type Void() { return { TypeKind::Void, nullptr}; }
    static Type array(Type t) {
        return { TypeKind::Array, std::make_shared<Type>(t) };
    }
};

struct RuntimeFunc {
    std::string name;
    Type retType;
    std::vector<Type> args;
};

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
    virtual Type visit(CastExpressionNode&) = 0;
    virtual Type visit(IndexExpressionNode&) = 0;
    
    static unsigned int typeSize(Type t) {
        switch (t.type) {
            case TypeKind::i64: return 8;
            case TypeKind::i32:     return 4;
            case TypeKind::Void: return 0;
            case TypeKind::Bool: return 8;
            case TypeKind::Array: return 8;
            case TypeKind::u8: return 1;
        }
        return 0;
    }

    static std::string typeName(Type t) {
        switch (t.type) {
            case TypeKind::i64: return "i32";
            case TypeKind::i32:     return "i32";
            case TypeKind::Void: return "unknown";
            case TypeKind::Bool: return "bool";
            case TypeKind::Array: return std::string("array of ")+std::string(typeName(*t.element));
            case TypeKind::u8: return "u8";
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
    
    std::vector<RuntimeFunc> rFuncs;
    
    bool isRuntimeFunc(const std::string& str) {
        for (auto& f : rFuncs)
            if (f.name.compare(str) == 0)
                return true;
        return false;
    }

    const RuntimeFunc& getRuntimeFunc(const std::string& name) {
        for (auto& f : rFuncs)
            if (f.name.compare(name) == 0)
                return f;
        exit(EXIT_FAILURE);
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
    bool isConst;
    LetStatementNode(int idx, Type type, std::unique_ptr<ExpressionNode> expr, bool isConst) : idx(idx), type(type), expr(std::move(expr)), isConst(isConst) {}
    Type accept(Visitor& v) override { return v.visit(*this); }
};

struct BlockStatementNode : StatementNode {
    std::vector<std::unique_ptr<StatementNode>> statements;
    BlockStatementNode(std::vector<std::unique_ptr<StatementNode>> statements) : statements(std::move(statements)) { }
    Type accept(Visitor& v) override { return v.visit(*this); }
};

struct ReassignStatementNode : StatementNode {
    std::unique_ptr<TermExpressionNode> lhs;
    std::unique_ptr<ExpressionNode> expr;
    ReassignStatementNode(std::unique_ptr<TermExpressionNode> lhs, std::unique_ptr<ExpressionNode> expr) : lhs(std::move(lhs)), expr(std::move(expr)) {  }
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
    Type inferredType = Type::Void();
    ExpressionNode(Type t) : inferredType(t) {}
    virtual ~ExpressionNode() = default;
    virtual Type accept(Visitor&) = 0;
};

struct TermExpressionNode : ExpressionNode {
    int value;
    Type type;
    bool isIdent;
    bool isConst;
    explicit TermExpressionNode(int v, bool isIdent, Type t, bool isConst) : ExpressionNode(t), value(v), type(t), isIdent(isIdent), isConst(isConst) {}
    Type accept(Visitor& v) override { return v.visit(*this); }
};

struct BinaryExpressionNode : ExpressionNode {
    std::unique_ptr<ExpressionNode> l;
    std::unique_ptr<ExpressionNode> r;
    BinOp op;
    BinaryExpressionNode(std::unique_ptr<ExpressionNode> l, BinOp op, std::unique_ptr<ExpressionNode> r) : ExpressionNode(Type::Void()), l(std::move(l)), r(std::move(r)), op(op) { }
    Type accept(Visitor& v) override { return v.visit(*this); }
};

struct CallExpressionNode : TermExpressionNode {
    std::vector<std::unique_ptr<ExpressionNode>> operands;
    CallExpressionNode(int v, bool isIdent, std::vector<std::unique_ptr<ExpressionNode>> operands, Type t) : TermExpressionNode(v, isIdent, t, false), operands(std::move(operands)) {}
    Type accept(Visitor& v) override { return v.visit(*this); }
};

struct CastExpressionNode : TermExpressionNode {
    std::unique_ptr<ExpressionNode> expr;
    Type type;
    CastExpressionNode(std::unique_ptr<ExpressionNode> expr, Type t) : TermExpressionNode(0, false, t, false), expr(std::move(expr)), type(t) {}
    Type accept(Visitor& v) override { return v.visit(*this); }
};

struct IndexExpressionNode : TermExpressionNode {
    std::unique_ptr<TermExpressionNode> primaryExpr;
    std::unique_ptr<ExpressionNode> indexExpr;
    IndexExpressionNode(std::unique_ptr<TermExpressionNode> prim, std::unique_ptr<ExpressionNode> index) : TermExpressionNode(prim->value, prim->isIdent, prim->type, false), primaryExpr(std::move(prim)), indexExpr(std::move(index)) {}
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
    std::vector<RuntimeFunc> rFunctions;
    std::vector<std::unique_ptr<FunctionNode>> functions;
    Type accept(Visitor& v) {
        return v.visit(*this);
    }
};
