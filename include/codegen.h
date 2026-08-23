#pragma once

#include "SymbolArena.h"
#include "ast.h"
#include "codegenemitter.h"
#include "diagnostics.h"
#include "stringtable.h"

class CodeGen : public Visitor {
    public:
    CodeGen(StringTable& strings, SymbolArena& arena, Diagnostics& diags, CodeGenEmitter& emitter) :
        strings_(strings), arena_(arena), diags_(diags), emitter_(emitter) {}
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

    unsigned int errors;
    private:

    unsigned int strLabels_ = 0;
    unsigned int continueLabel_ = 0;
    unsigned int breakLabel_ = 0;
    void emitStore(const std::string& reg, unsigned int width);
    void emitLoad(const std::string& reg, const Type& t);
    void emitAddress(Expression& e);

    StringTable& strings_;
    SymbolArena& arena_;
    Diagnostics& diags_;
    CodeGenEmitter& emitter_;

    FunctionDecl* currentFn_ = nullptr;
};

