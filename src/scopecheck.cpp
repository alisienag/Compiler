#include "scopecheck.h"
#include "ast.h"
#include <algorithm>
#include <iostream>

Type ScopeCheck::visit(ProgramNode& p) {
    this->rFuncs = p.rFunctions;
    this->errors = 0;
    enterScope();
    for (auto & funcs : p.functions)
        scopes_.back().push_back(funcs->nameIdx);
    for (auto& funcs : p.functions) {
        funcs->accept(*this);
    }
    return Type::Void();
}
Type ScopeCheck::visit(FunctionNode& p) {
    initCounter_ = 0;
    enterScope();
    scopes_.back().push_back(p.nameIdx);
    for (auto& op : p.operands) {
        op->accept(*this);
    }
    p.statement->accept(*this);
    exitScope();
    p.localCount = initCounter_;
    return Type::Void();
}

Type ScopeCheck::visit(OperandNode& o) {
    if (existsInCurrentScope(o.identIdx)) {
        std::cerr << "ScopeCheck Error: cannot redeclare operand " << table_.findStringByIdx(o.identIdx) << "\n";
        errors++;
    }
    scopes_.back().push_back(o.identIdx);
    return Type::Void();
}

Type ScopeCheck::visit(LetStatementNode& s) {
    unsigned int currentIdx = s.idx;
    if (std::find(scopes_.back().begin(), scopes_.back().end(), currentIdx) != scopes_.back().end()) {
        std::cerr << "Scopechecking Error: Cannot redeclare variable " << table_.findStringByIdx(currentIdx) << "\n";
        errors++;
    } else {
        scopes_.back().push_back(currentIdx);
        if (s.isConst) {
            const_.back().push_back(currentIdx);
        }
        initCounter_++;
    }
    return Type::Void();
}
Type ScopeCheck::visit(BlockStatementNode& s) {
    enterScope();
    for (auto& stmt : s.statements) {
        stmt->accept(*this);
    }
    exitScope();
    return Type::Void();
}

Type ScopeCheck::visit(ReassignStatementNode& s) {
    int scopeLayer = existsAtAll(this->scopes_, s.lhs->value);
    if (scopeLayer == 0) {
        std::cerr << "ScopeCheck: identifier " << table_.findStringByIdx(s.lhs->value) << " not defined to reassign!\n";
        errors++;
    }
    int constLayer = existsAtAll(this->const_, s.lhs->value);
    if (constLayer) {
        if (constLayer >= scopeLayer) {
            std::cerr << "ScopeCheck: identifier " << table_.findStringByIdx(s.lhs->value) << " is declared const and can't be reassigned!!\n";
            errors++;
        }
    }
    s.expr->accept(*this);
    return Type::Void();
}

Type ScopeCheck::visit(ReturnStatementNode& s) {
    s.expr->accept(*this);
    return Type::Void();
}

Type ScopeCheck::visit(ExpressionStatementNode& s) {
    s.expr->accept(*this);
    return Type::Void();
}

Type ScopeCheck::visit(IfStatementNode& s) {
    s.cond->accept(*this);
    scopes_.push_back(std::vector<int>());
    s.ifNode->accept(*this);
    scopes_.pop_back();
    if (s.hasElse) {
        scopes_.push_back(std::vector<int>());
        s.elseNode->accept(*this);
        scopes_.pop_back();
    }
    return Type::Void();
}

Type ScopeCheck::visit(BinaryExpressionNode& e) {
    e.l->accept(*this);
    e.r->accept(*this);
    return Type::Void();
}

Type ScopeCheck::visit(TermExpressionNode& e) {
    if (e.isIdent) {
        if (existsAtAll(this->scopes_, e.value) == 0) {
            std::cerr << "ScopeCheck Error: identifier " << table_.findStringByIdx(e.value) << " not defined to use!\n";
            errors++;
        }
    }
    return Type::Void();
}

Type ScopeCheck::visit(CallExpressionNode& e) {
    if (existsAtAll(this->scopes_, e.value) == 0) {
        if (isRuntimeFunc(table_.findStringByIdx(e.value))) {
            return Type::Void();
        }
        std::cerr << "ScopeCheck Error: cannot find function " << e.value << "\n";
        errors++;
    }
    return Type::Void();
}

Type ScopeCheck::visit(CastExpressionNode& e) {
    e.expr->accept(*this);
    return Type::Void();
}

Type ScopeCheck::visit(IndexExpressionNode& e) {
    e.primaryExpr->accept(*this);
    e.indexExpr->accept(*this);
    return Type::Void();
}
