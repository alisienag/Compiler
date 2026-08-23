#include "typecheck.h"

void TypeCheck::visit(Program& p) {
    for (auto& f : p.functions) {
        f->accept(*this);
    }
}

void TypeCheck::visit(FunctionDecl& f) {
    if (f.isExtern()) return;
    currentFn_ = &f;
    for (auto& p : f.params) {
        p->accept(*this);
    }
    f.body->accept(*this);
}

void TypeCheck::visit(Param&) {
}

void TypeCheck::visit(LetStmt& s) {
    check(*s.init, s.declared);
    Type initType = s.init->type;
    if (initType == Type::voidType()) {
        diags_.error(s.span, "cannot bind 'void' to a variable");
    }
    if (s.declared == Type::unresolved()) {
        s.declared = initType;
        if (s.symbol)
            s.symbol->type = initType;
    }
    if (initType != Type::unresolved() && initType != s.declared) {
        diags_.error(s.span, "Variable " + strings_.getName(s.nameIdx) + " of type "
                + typeName(s.declared) + " cannot be assigned to expression of type"
                + typeName(initType));
    }
}

void TypeCheck::visit(ExprStmt& s) {
    if (s.target) {
        infer(*s.target);
        check(*s.value, s.target->type);
        if (s.target->type != Type::unresolved() && s.value->type != s.target->type)
            diags_.error(s.span, "cannot assign " + typeName(s.value->type) + 
                    " to " + typeName(s.target->type));
    } else {
        infer(*s.value);       
    }
}

void TypeCheck::visit(ReturnStmt& s) {
    if (!currentFn_) return;
    if (!s.value) {
        if (currentFn_->returnType != Type::voidType()) {
            diags_.error(s.span, "'ret' with no value in a function returning " + typeName(currentFn_->returnType));
        }
        return;
    }
    check(*s.value, currentFn_->returnType);
    if (s.value->type != Type::unresolved() && s.value->type != currentFn_->returnType) {
        diags_.error(s.span, "Cannot return type " + typeName(s.value->type) +
                " on function with return type " + typeName(currentFn_->returnType));
    }
}

void TypeCheck::visit(IfStmt& s) {
    infer(*s.cond);
    if (s.cond->type == Type::voidType()) {
        diags_.error(s.span, "condition for if statement has no value ('void')");
    }
    s.thenBlock->accept(*this);
    if (s.hasElse()) {
        s.elseBranch->accept(*this);
    }
}

void TypeCheck::visit(WhileStmt& s) {
    infer(*s.cond);
    if (s.cond->type == Type::voidType()) {
        diags_.error(s.span, "condition for while statement has no value ('void')");
    }
    s.body->accept(*this);
}

void TypeCheck::visit(BreakStmt&) {

}

void TypeCheck::visit(ContinueStmt&) {

}

void TypeCheck::visit(BlockStmt& s) {
    for (auto& stmt : s.statements) {
        stmt->accept(*this);
    }
}


void TypeCheck::visit(CallExpr& e) {
    infer(*e.callee);
    auto* name = dynamic_cast<NameExpr*>(e.callee.get());
    Symbol* fn = name ? name->symbol : nullptr;
    if (!fn || fn->storage != Storage::Function) {
        diags_.error(e.span, "expression not callable");
        for (auto& arg : e.args)
            infer(*arg);
        e.type = Type::unresolved();
        return;
    }
    if (e.args.size() != fn->paramTypes.size()) {
        diags_.error(e.span, fn->name + " expects " +
                std::to_string(fn->paramTypes.size()) + " arguments, got "
                + std::to_string(e.args.size()));
    }
    for (size_t i = 0; i < e.args.size(); i++) {
        if (i < fn->paramTypes.size()) {
            check(*e.args[i], fn->paramTypes[i]);
            if (e.args[i]->type != fn->paramTypes[i]) {
                diags_.error(e.args[i]->span, "argument " + std::to_string(i+1)
                        + " is of type " + typeName(e.args[i]->type) + ", expected "
                        + typeName(fn->paramTypes[i]));
            }
        } else {
            infer(*e.args[i]);
        }
    }
    e.type = fn->type;
}

void TypeCheck::visit(IndexExpr& e) {
    infer(*e.index);
    if (e.index->type != Type::unresolved() && !e.index->type.isInteger()) {
        diags_.error(e.span, "index expression must be an integer, not " + typeName(e.index->type));
    }
    infer(*e.base);
    if (!e.base->type.isArray()) {
        diags_.error(e.span, "cannot index value of type " + typeName(e.base->type));
        e.type = Type::unresolved();
        return;
    }
    e.type = *e.base->type.element;
}

void TypeCheck::visit(CastExpr& e) {
    infer(*e.operand);
    e.type = e.target;
}

void TypeCheck::visit(LogicalExpr& e) {
    infer(*e.lhs);
    infer(*e.rhs);
    if (e.lhs->type == Type::unresolved() || e.rhs->type == Type::unresolved()) {
        e.type = Type::unresolved();
        return;
    }
    if (e.lhs->type == Type::voidType() || e.rhs->type == Type::voidType()) {
        diags_.error(e.span, "operand of '" + std::string(opName(e.op))
                + "' has no value ('void')");
    }
    e.type = Type::boolType();
}

void TypeCheck::visit(BinaryExpr& e) {
    infer(*e.lhs);
    check(*e.rhs, e.lhs->type);
    if (e.lhs->type != e.rhs->type)
        check(*e.lhs, e.rhs->type);
    
    if (e.lhs->type == Type::unresolved() || e.rhs->type == Type::unresolved()) {
        e.type = Type::unresolved();
        return;
    }
    bool ok = true;
    if (!e.lhs->type.isInteger()) {
        diags_.error(e.span, "left hand side binary operation must be of type integer");
        ok = false;
    }
    if (!e.rhs->type.isInteger()) {
        diags_.error(e.span, "right hand side binary operation must be of type integer");
        ok = false;
    }
    if (ok && e.lhs->type != e.rhs->type) {
        diags_.error(e.span, "cannot perform '" + std::string(opName(e.op)) + "' between types "
                + typeName(e.lhs->type) + " and " +  typeName(e.rhs->type));
    }
    e.type = ok ? (isComparison(e.op) ? Type::boolType() : e.lhs->type) : Type::unresolved();
}

void TypeCheck::visit(UnaryExpr& e) {
    if (e.op == UnaryOp::Not) {
        infer(*e.operand);
        if (e.operand->type == Type::unresolved()) {
            e.type = Type::unresolved();
            return;
        }
        if (e.operand->type == Type::voidType()) {
            diags_.error(e.span, "cannot perform ! on 'void' type");
        }
        e.type = Type::boolType();
        return;
    }
    check(*e.operand, expected_);
    if (e.operand->type == Type::unresolved()) {
        e.type = Type::unresolved();
        return;
    }
    if (!e.operand->type.isInteger()) {
        diags_.error(e.span, "cannot negate a value of type " + typeName(e.operand->type));
        e.type = Type::unresolved();
        return;
    }
    e.type = e.operand->type;
}

void TypeCheck::visit(NameExpr& e) {
    if (!e.symbol) {
        e.type = Type::unresolved(); 
        return;
    }
    e.type = e.symbol->type;
}

void TypeCheck::visit(StringLiteral& e) {
    e.type = Type::array(Type::u8());
}

void TypeCheck::visit(BoolLiteral& e) {
    e.type = Type::boolType();
}

void TypeCheck::visit(IntLiteral& e) {
    e.type = expected_.isInteger() ? expected_ : Type::i32();
}
