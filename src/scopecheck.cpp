#include "scopecheck.h"
#include "ast.h"
#include <algorithm>
#include <iostream>

void ScopeCheck::visit(ProgramNode& p) {
    scopes_.push_back(std::vector<int>());
    for (auto & funcs : p.functions)
        scopes_.back().push_back(funcs->nameIdx);
    for (auto& funcs : p.functions) {
        funcs->accept(*this);
    }
}
void ScopeCheck::visit(FunctionNode& p) {
    initCounter_ = 0;
    scopes_.push_back({});
    scopes_.back().push_back(p.nameIdx);
    for (auto& op : p.operands) {
        op->accept(*this);
    }
    p.statement->accept(*this);
    scopes_.pop_back();
    p.localCount = initCounter_;
}

void ScopeCheck::visit(OperandNode& o) {
    if (existsInCurrentScope(o.identIdx)) {
        std::cerr << "ScopeCheck Error: cannot redeclare operand " << table_.findStringByIdx(o.identIdx) << "\n";
    }
    scopes_.back().push_back(o.identIdx);
}

void ScopeCheck::visit(LetStatementNode& s) {
    unsigned int currentIdx = s.idx;
    if (std::find(scopes_.back().begin(), scopes_.back().end(), currentIdx) != scopes_.back().end()) {
        std::cerr << "Scopechecking Error: Cannot redeclare variable " << table_.findStringByIdx(currentIdx) << "\n";
    } else {
        scopes_.back().push_back(currentIdx);
        initCounter_++;
    }
}
void ScopeCheck::visit(BlockStatementNode& s) {
    scopes_.push_back(std::vector<int>());
    for (auto& stmt : s.statements) {
        stmt->accept(*this);
    }
    scopes_.pop_back();
}

void ScopeCheck::visit(ReassignStatementNode& s) {
    if (!existsAtAll(s.idx)) {
        std::cerr << "ScopeCheck: identifier " << table_.findStringByIdx(s.idx) << " not defined to reassign!\n";
    }
    s.expr->accept(*this);
}

void ScopeCheck::visit(ReturnStatementNode& s) {
    s.expr->accept(*this);
}

void ScopeCheck::visit(ExpressionStatementNode& s) {
    s.expr->accept(*this);
}

void ScopeCheck::visit(BinaryExpressionNode& e) {
    e.l->accept(*this);
    e.r->accept(*this);
}

void ScopeCheck::visit(TermExpressionNode& e) {
    if (e.isIdent) {
        if (!existsAtAll(e.value)) {
            std::cerr << "ScopeCheck Error: identifier " << table_.findStringByIdx(e.value) << " not defined to use!\n";
        }
    }
}

void ScopeCheck::visit(CallExpressionNode& e) {
    if (!existsAtAll(e.value)) {
        std::cerr << "ScopeCheck Error: cannot find function " << e.value << "\n";
    }
}

bool ScopeCheck::existsInCurrentScope(int idx) {
    if (std::find(scopes_.back().begin(), scopes_.back().end(), idx) != scopes_.back().end()) {
        return true;
    } else {
        return false;
    }

}

bool ScopeCheck::existsAtAll(int idx) {
    for (auto& v : this->scopes_) {
        if (std::find(v.begin(), v.end(), idx) != v.end()) {
            return true;
        }
    }
    return false;
}
