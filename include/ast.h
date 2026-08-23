#pragma once

#include <memory>
#include <vector>
#include <string>

// Source Spans

struct Span {
    unsigned int line = 0;
    unsigned int col = 0;
    unsigned int length = 0;
};

// Types

enum class TypeKind {
    Unresolved, Void,
    i64, u64, i32, u32, i16, u16, i8, u8,
    Bool,
    Array
};

struct Type {
    TypeKind kind = TypeKind::Unresolved;
    std::shared_ptr<Type> element;
    
    static Type unresolved() { return { TypeKind::Unresolved, nullptr }; }
    static Type voidType() { return { TypeKind::Void, nullptr }; }
    static Type i64() { return { TypeKind::i64, nullptr}; }
    static Type u64() { return { TypeKind::u64, nullptr}; }
    static Type i32() { return { TypeKind::i32, nullptr}; }
    static Type u32() { return { TypeKind::u32, nullptr}; }
    static Type i16() { return { TypeKind::i16, nullptr}; }
    static Type u16() { return { TypeKind::u16, nullptr}; }
    static Type i8() { return { TypeKind::i8, nullptr }; }
    static Type u8() { return { TypeKind::u8, nullptr }; }
    static Type boolType() { return { TypeKind::Bool, nullptr}; }
    static Type array(Type t) {
        return { TypeKind::Array, std::make_shared<Type>(t) };
    }

    bool isArray() const { return kind == TypeKind::Array; }
    bool isInteger() const {
        switch (kind) {
            case TypeKind::i64: case TypeKind::u64:
            case TypeKind::i32: case TypeKind::u32:
            case TypeKind::i16: case TypeKind::u16:
            case TypeKind::i8: case TypeKind::u8:
            return true;
            default: return false;
        }
    }
    bool isSigned() const {
        switch(kind) {
            case TypeKind::i64: case TypeKind::i32:
            case TypeKind::i16: case TypeKind::i8:
            return true;
            default: return false;
        }
    }
};

inline bool operator==(const Type& a, const Type& b) {
    if (a.kind != b.kind) { return false; }
    if (a.kind != TypeKind::Array) { return true; }
    if (!a.element || !b.element) return a.element == b.element;
    return *a.element == *b.element;
}

inline bool operator!=(const Type& a, const Type& b) { return !(a==b); }

inline unsigned int sizeOf(const Type& t) {
    switch(t.kind) {
        case TypeKind::i64: case TypeKind::u64:
        case TypeKind::Array:
        return 8;
        case TypeKind::i32: case TypeKind::u32:
        return 4;
        case TypeKind::i16: case TypeKind::u16:
        return 2;
        case TypeKind::i8: case TypeKind::u8:
        case TypeKind::Bool:
        return 1;
        case TypeKind::Void: case TypeKind::Unresolved:
        return 0;
    }
}

inline std::string typeName(const Type& t) {
    switch (t.kind) {
        case TypeKind::i64:
            return "'i64'";
        case TypeKind::u64:
            return "'u64'";
        case TypeKind::Array:
            return "'[" + typeName(*t.element) + "]";
        case TypeKind::i32:
            return "'i32'";
        case TypeKind::u32:
            return "'u32'";
        case TypeKind::i16:
            return "'i16'";
        case TypeKind::u16:
            return "'u16'";
        case TypeKind::i8:
            return "'i8'";
        case TypeKind::u8:
            return "'u8'";
        case TypeKind::Bool:
            return "'bool'";
        case TypeKind::Void:
            return "'void'";
        case TypeKind::Unresolved:
            return "'unresolved'";
        return 0;
    }
}

//Operators

enum class BinOp { Add, Sub, Mul, Div, Eq, Ne, Lt, Gt, Le, Ge };
enum class LogicalOp { And, Or};
enum class UnaryOp { Neg, Not };

inline const char* opName(BinOp op) {
    switch(op) {
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
}

inline const char* opName(LogicalOp op) {
    switch(op) {
        case LogicalOp::And: return "&&";
        case LogicalOp::Or: return "||";
    }
}

inline const char* opName(UnaryOp op) {
    switch(op) {
        case UnaryOp::Neg: return "-";
        case UnaryOp::Not: return "!";
    }
}

inline bool isComparison(BinOp op) {
    switch(op) {
        case BinOp::Eq: case BinOp::Ne:
        case BinOp::Lt: case BinOp::Gt:
        case BinOp::Le: case BinOp::Ge:
        return true;
        default: return false;
    }
}

// Symbols

enum class Storage { Local, Param, Function };

struct Symbol {
    std::string name;
    Type type;
    Storage storage = Storage::Local;
    std::string asmName;
    bool isMutable = false;
    int offset = 0;
    std::vector<Type> paramTypes;
    Span declaredAt;
};

// Ast Nodes

struct Program;
struct FunctionDecl;
struct Param;

struct LetStmt;
struct ExprStmt;
struct ReturnStmt;
struct IfStmt;
struct WhileStmt;
struct BreakStmt;
struct ContinueStmt;
struct BlockStmt;

struct CallExpr;
struct IndexExpr;
struct CastExpr;
struct LogicalExpr;
struct BinaryExpr;
struct UnaryExpr;
struct NameExpr;
struct StringLiteral;
struct BoolLiteral;
struct IntLiteral;


class Visitor {
    public:
    virtual ~Visitor() = default;
    virtual void visit(Program&) = 0;
    virtual void visit(FunctionDecl&) = 0;
    virtual void visit(Param&) = 0;

    virtual void visit(LetStmt&) = 0;
    virtual void visit(ExprStmt&) = 0;
    virtual void visit(ReturnStmt&) = 0;
    virtual void visit(IfStmt&) = 0;
    virtual void visit(WhileStmt&) = 0;
    virtual void visit(BreakStmt&) = 0;
    virtual void visit(ContinueStmt&) = 0;
    virtual void visit(BlockStmt&) = 0;

    virtual void visit(CallExpr&) = 0;
    virtual void visit(IndexExpr&) = 0;
    virtual void visit(CastExpr&) = 0;
    virtual void visit(LogicalExpr&) = 0;
    virtual void visit(BinaryExpr&) = 0;
    virtual void visit(UnaryExpr&) = 0;
    virtual void visit(NameExpr&) = 0;
    virtual void visit(StringLiteral&) = 0;
    virtual void visit(BoolLiteral&) = 0;
    virtual void visit(IntLiteral&) = 0;
       
};

// Base Nodes

struct Node {
    Span span;
    virtual ~Node() = default;
    virtual void accept(Visitor&) = 0;
};

struct Expression : Node {
    Type type;
    bool isLvalue = false;
};

struct Statement : Node {};

using StmtPtr = std::unique_ptr<Statement>;
using ExprPtr = std::unique_ptr<Expression>;

#define ACCEPT void accept(Visitor& v) override { v.visit(*this); }

// Declaration Nodes

struct Program : Node {
    std::vector<std::unique_ptr<FunctionDecl>> functions;
    ACCEPT
};

struct FunctionDecl : Node {
    int nameIdx;
    Type returnType;
    std::vector<std::unique_ptr<Param>> params;
    std::unique_ptr<BlockStmt> body;
    std::string asmName;
    Symbol* symbol = nullptr;
    unsigned int frameSize = 0;
    bool isExtern() const { return body == nullptr; }
    ACCEPT
};

struct Param : Node {
    int nameIdx;
    Type type;
    bool isMutable = false;
    Symbol* symbol = nullptr;
    Param(int idx, Type t, bool mut) : nameIdx(idx), type(t), isMutable(mut) {}
    ACCEPT
};

// Statement Nodes

struct LetStmt : Statement {
    int nameIdx;
    Type declared;
    ExprPtr init;
    bool isMutable = false;
    Symbol* symbol = nullptr;
    LetStmt(int idx, Type d, ExprPtr e, bool mut)
        : nameIdx(idx), declared(d), init(std::move(e)), isMutable(mut) {}
    ACCEPT
};

struct ExprStmt : Statement {
    ExprPtr target;
    ExprPtr value;
    ExprStmt(ExprPtr t, ExprPtr v) : target(std::move(t)), value(std::move(v)) {}
    bool isAssignment() const { return target != nullptr; }
    ACCEPT
};

struct ReturnStmt : Statement {
    ExprPtr value;
    explicit ReturnStmt(ExprPtr v) : value(std::move(v)) {}
    ACCEPT
};

struct BlockStmt : Statement {
    std::vector<StmtPtr> statements;
    explicit BlockStmt(std::vector<StmtPtr> s) : statements(std::move(s)) {}
    ACCEPT
};

struct IfStmt : Statement {
    ExprPtr cond;
    std::unique_ptr<BlockStmt> thenBlock;
    StmtPtr elseBranch;
    IfStmt(ExprPtr c, std::unique_ptr<BlockStmt> t, StmtPtr e)
        : cond(std::move(c)), thenBlock(std::move(t)), elseBranch(std::move(e)) {}
    bool hasElse() const { return elseBranch != nullptr; }
    ACCEPT
};

struct WhileStmt : Statement {
    ExprPtr cond;
    std::unique_ptr<BlockStmt> body;
    WhileStmt(ExprPtr c, std::unique_ptr<BlockStmt> b)
        : cond(std::move(c)), body(std::move(b)) {}
    ACCEPT
};

struct BreakStmt : Statement { ACCEPT };
struct ContinueStmt : Statement { ACCEPT };

// Expression Nodes

struct CallExpr : Expression {
    ExprPtr callee;
    std::vector<ExprPtr> args;
    CallExpr(ExprPtr c, std::vector<ExprPtr> a)
        : callee(std::move(c)), args(std::move(a)) {}
    ACCEPT
};

struct IndexExpr : Expression {
    ExprPtr base, index;
    IndexExpr(ExprPtr b, ExprPtr i) : base(std::move(b)), index(std::move(i)) {
        this->isLvalue = true;
    }
    ACCEPT
};

struct CastExpr : Expression {
    ExprPtr operand;
    Type target;
    CastExpr(ExprPtr o, Type t) : operand(std::move(o)), target(t) {}
    ACCEPT
};

struct LogicalExpr : Expression {
    LogicalOp op;
    ExprPtr lhs, rhs;
    LogicalExpr(ExprPtr l, LogicalOp op, ExprPtr r)
        : op(op), lhs(std::move(l)), rhs(std::move(r)) {}
    ACCEPT
};

struct BinaryExpr : Expression {
    BinOp op;
    ExprPtr lhs, rhs;
    BinaryExpr(ExprPtr l, BinOp op, ExprPtr r)
        : op(op), lhs(std::move(l)), rhs(std::move(r)) {}
    ACCEPT
};

struct UnaryExpr : Expression {
    UnaryOp op;
    ExprPtr operand;
    UnaryExpr(UnaryOp op, ExprPtr o) : op(op), operand(std::move(o)) {}
    ACCEPT
};

struct NameExpr : Expression {
    int nameIdx;
    Symbol* symbol = nullptr;
    explicit NameExpr(int idx) : nameIdx(idx) { isLvalue = true; }
    ACCEPT
};

struct StringLiteral : Expression {
    std::string value;
    int label = -1;
    explicit StringLiteral(std::string v) : value(std::move(v)) {}
    ACCEPT
};

struct BoolLiteral : Expression {
    bool value = false;
    explicit BoolLiteral(bool v) : value(v) {}
    ACCEPT
};

struct IntLiteral : Expression {
    long long value = 0;
    explicit IntLiteral(long long v) : value(v) {}
    ACCEPT
};

#undef ACCEPT
