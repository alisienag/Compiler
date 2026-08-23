#include "scopecheck.h"
#include "ast.h"

void ScopeCheck::visit(Program& p) {
    for (auto& f : p.functions) {
        auto it = globals_.find(f->nameIdx);
        if (it != globals_.end()) {
            diags_.error(f->span, "function '" + strings_.getName(f->nameIdx)
                    + "' is already defined!");
            f->symbol = it->second;
            continue;
        }

        Symbol* s = arena_.make();
        s->name = strings_.getName(f->nameIdx);
        s->type = f->returnType;
        s->storage = Storage::Function;
        s->isMutable = false;
        s->declaredAt = f->span;
        s->asmName = f->asmName;

        for (auto& param : f->params)
            s->paramTypes.push_back(param->type);
        f->symbol = s;
        globals_.emplace(f->nameIdx, s);
    }
    for (auto& f : p.functions) {
        f->accept(*this);
    }
}

void ScopeCheck::visit(FunctionDecl& f) {
    if (f.isExtern()) return;
    
    currentFn_ = &f;
    localSlots_ = 0;
    paramSlots_ = 0;
    loopDepth_ = 0;

    pushScope();
    for (auto& p : f.params) {
        p->accept(*this);
    }

    f.body->accept(*this);

    popScope();

    f.frameSize = 8 * localSlots_;
    currentFn_ = nullptr;
}

void ScopeCheck::visit(Param& p) {
    Symbol* s = declare(p.nameIdx, p.type, p.isMutable, p.span);
    if (!s) return;
    s->storage = Storage::Param;

    assignParamSlot(*s);

    p.symbol = s;
}

// Statements

void ScopeCheck::visit(LetStmt& s) {
    s.init->accept(*this);

    Symbol * sym = declare(s.nameIdx, s.declared, s.isMutable, s.span);
    if (!sym) return;
    
    sym->storage = Storage::Local;
    assignLocalSlot(*sym);

    s.symbol = sym;
}

void ScopeCheck::visit(ExprStmt& s) {
    if (s.target) {
        s.target->accept(*this);
        NameExpr* root = rootName(s.target.get());
        if (!root) {
            diags_.error(s.target->span, "cannot assign to a temporary");
        } else if (root->symbol && !root->symbol->isMutable) {
            diags_.error(s.target->span, "cannot assign to immutable '"
                    + strings_.getName(root->nameIdx) + "';");
        }
    }
    s.value->accept(*this);
}

void ScopeCheck::visit(ReturnStmt& s) {
    if (s.value)
        s.value->accept(*this);
}

void ScopeCheck::visit(IfStmt& s) {
    s.cond->accept(*this);
    
    s.thenBlock->accept(*this);
    
    if (s.hasElse())
        s.elseBranch->accept(*this);
}

void ScopeCheck::visit(WhileStmt& s) {
    s.cond->accept(*this);
    loopDepth_++;
    s.body->accept(*this);
    loopDepth_--;
}

void ScopeCheck::visit(BreakStmt& s) {
    if (!loopDepth_)
        diags_.error(s.span, "'break' outside of loop");
}

void ScopeCheck::visit(ContinueStmt& s) {
    if (!loopDepth_)
        diags_.error(s.span, "'continue' outside of loop");
}

void ScopeCheck::visit(BlockStmt& s) {
    pushScope();
    for (auto& stmt : s.statements) {
        stmt->accept(*this);
    }
    popScope();
}

// Expressions

void ScopeCheck::visit(CallExpr& e) {
    e.callee->accept(*this);

    for (auto& args : e.args) {
        args->accept(*this);
    }
}

void ScopeCheck::visit(IndexExpr& e) {
    e.base->accept(*this);
    e.index->accept(*this);
}

void ScopeCheck::visit(CastExpr& e) {
    e.operand->accept(*this);
}

void ScopeCheck::visit(LogicalExpr& e) {
    e.lhs->accept(*this);
    e.rhs->accept(*this);
}

void ScopeCheck::visit(BinaryExpr& e) {
    e.lhs->accept(*this);
    e.rhs->accept(*this);
}

void ScopeCheck::visit(UnaryExpr& e) {
    e.operand->accept(*this);
}

void ScopeCheck::visit(NameExpr& e) {
    Symbol* s = lookup(e.nameIdx);
    if (!s) {
        diags_.error(e.span, "undeclared identifier '"
                + strings_.getName(e.nameIdx) + "'");
        return;
    }
    e.symbol = s;
}

void ScopeCheck::visit(StringLiteral&) {
    
}

void ScopeCheck::visit(BoolLiteral&) {
    
}

void ScopeCheck::visit(IntLiteral&) {
    
}

Symbol* ScopeCheck::declare(int nameIdx,  Type type, bool isMutable, Span at) {
    Scope& scope = scopes_.back();
    
    auto it = scope.find(nameIdx);
    if (it != scope.end()) {
        diags_.error(at, "'" + strings_.getName(nameIdx)
                + "' is already declared in this scope");
        return nullptr;
    }
    Symbol* symbol = arena_.make();
    symbol->name = strings_.getName(nameIdx);
    symbol->type = type;
    symbol->isMutable = isMutable;
    symbol->declaredAt = at;
    
    scope.emplace(nameIdx, symbol);
    return symbol;
}

Symbol* ScopeCheck::lookup(int nameIdx) const {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto found = it->find(nameIdx);
        if (found != it->end()) return found->second;
    }
    const auto it = globals_.find(nameIdx);
    if (it != globals_.end()) {
        return it->second;
    }
    return nullptr;
}

void ScopeCheck::assignLocalSlot(Symbol& s) {
    s.offset = -8 * static_cast<int>(++localSlots_);
}

void ScopeCheck::assignParamSlot(Symbol& s) {
    s.offset = 16 + (8 * static_cast<int>(paramSlots_++));
}

NameExpr* ScopeCheck::rootName(Expression* e) {
    for (;;) {
        if (auto* n = dynamic_cast<NameExpr*>(e))
            return n;
        if (auto* ix = dynamic_cast<IndexExpr*>(e)) {
            e = ix->base.get();
            continue;
        }
        return nullptr;
    }
}
