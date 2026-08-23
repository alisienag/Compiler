#pragma once

#include "SymbolArena.h"
#include "ast.h"
#include "diagnostics.h"
#include "stringtable.h"

class ScopeCheck : public Visitor {
    public:
    ScopeCheck(StringTable& strings, SymbolArena& arena, Diagnostics& diags)
        : strings_(strings), arena_(arena), diags_(diags) {}
    virtual void visit(Program&) override;
    virtual void visit(FunctionDecl&) override;
    virtual void visit(Param&) override;

    virtual void visit(LetStmt&) override;
    virtual void visit(ExprStmt&) override;
    virtual void visit(ReturnStmt&) override;
    virtual void visit(IfStmt&) override;
    virtual void visit(WhileStmt&) override;
    virtual void visit(BreakStmt&) override;
    virtual void visit(ContinueStmt&) override;
    virtual void visit(BlockStmt&) override;

    virtual void visit(CallExpr&) override;
    virtual void visit(IndexExpr&) override;
    virtual void visit(CastExpr&) override;
    virtual void visit(LogicalExpr&) override;
    virtual void visit(BinaryExpr&) override;
    virtual void visit(UnaryExpr&) override;
    virtual void visit(NameExpr&) override;
    virtual void visit(StringLiteral&) override;
    virtual void visit(BoolLiteral&) override;
    virtual void visit(IntLiteral&) override;
    private:
    using Scope = std::unordered_map<int, Symbol*>;

    void pushScope() { scopes_.emplace_back(); }
    void popScope() { scopes_.pop_back(); }

    Symbol* declare(int nameIdx, Type type, bool isMutable, Span at);
    Symbol* lookup(int nameIdx) const;
    void assignLocalSlot(Symbol& s);
    void assignParamSlot(Symbol& s);

    static NameExpr* rootName(Expression* e);

    StringTable& strings_;
    SymbolArena& arena_;
    Diagnostics& diags_;

    Scope globals_;
    std::vector<Scope> scopes_;
    FunctionDecl* currentFn_ = nullptr;
    unsigned int localSlots_ = 0;
    unsigned int paramSlots_ = 0;
    int loopDepth_ = 0;
};
