#include "typecheck.h"
#include "ast.h"
#include <iostream>

Type TypeCheck::visit(ProgramNode& p) {
    this->rFuncs = p.rFunctions;
    errors = 0;
    for (auto& funcs : p.functions) {
        funcs->accept(*this);
    }
    return Type::Void();
}
Type TypeCheck::visit(FunctionNode& f) {
    funcType[f.nameIdx] = f.type;
    currentScope++;
    for (auto& op : f.operands)
        funcOpCount[f.nameIdx].push_back(op->accept(*this));
    Type stmtType = f.statement->accept(*this);
    if (!conforms(f.type, stmtType)) {
        std::cerr << "TypeCheck Error: function " << table_.findStringByIdx(f.nameIdx) << " of return type " << typeName(f.type) << " cannot return type of " << typeName(stmtType) << "\n";
        errors++;
    }
    currentScope--;
    return f.type;
}
Type TypeCheck::visit(OperandNode& op) {
    varType[currentScope][op.identIdx] = op.type;
    return op.type;
}

Type TypeCheck::visit(LetStatementNode& s) {
    Type exprType = s.expr->accept(*this);
    if (!conforms(s.type, exprType)) {
        std::cerr << "TypeCheck Error: cannot assign type " << typeName(exprType) << " to variable of type " << typeName(s.type) << "\n";
        errors++;
    }
    varType[currentScope][s.idx] = s.type;
    return Type::Void();
}
Type TypeCheck::visit(BlockStatementNode& s) {
    Type type = Type::Void();
    currentScope++;
    for (auto& stmt : s.statements) {
        Type stmtType = stmt->accept(*this);
        if (stmtType.type != TypeKind::Void) { // all statements return Unknown except for the return statement!
            type = stmtType;
        }
    }
    currentScope--;
    return type;
}

Type TypeCheck::visit(ReassignStatementNode& s) {
    Type lhsType = s.lhs->accept(*this);
    Type exprType = s.expr->accept(*this);
    if (!conforms(exprType, lhsType)) {
        std::cerr << "TypeCheck Error: cannot reassign type " << typeName(exprType) << " to variable of type " << typeName(getVarType(s.lhs->value));
        errors++;
    }
    return Type::Void();
}

Type TypeCheck::visit(ReturnStatementNode& s) {
    return s.expr->accept(*this);
}
Type TypeCheck::visit(ExpressionStatementNode& s) {
    s.expr->accept(*this);
    return Type::Void();
}

Type TypeCheck::visit(IfStatementNode& s) {
    Type condType = s.cond->accept(*this);
    if (condType.type != TypeKind::Bool) {
        std::cerr << "TypeCheck Error: expected an expression of type bool for if\n";
        errors++;
    }
    Type ifType = s.ifNode->accept(*this);
    Type elseType = Type::Void();
    if (s.hasElse) {
        elseType = s.elseNode->accept(*this);
        if (ifType.type != elseType.type) {
            std::cout << "TypeCheck warning: if and else branches dont return the same type!\n";
        }
    }
    return ifType;
}

Type TypeCheck::visit(BinaryExpressionNode& e) {
    Type lType = e.l->accept(*this);
    Type rType = e.r->accept(*this);
    if (lType.type != rType.type) {
        std::cerr << "TypeCheck Error: cannot perform " << opName(e.op) << " operation on differing types!\n";
        errors++;
    }
    if (e.op == BinOp::Eq) {
        e.inferredType.type = TypeKind::Bool;
        return Type::Bool();
    }
    if (lType.type == TypeKind::i32) {
        e.inferredType = Type::i32();
    }
    return lType;
}
Type TypeCheck::visit(TermExpressionNode& e) {
    if (e.isIdent) {
        return getVarType(e.value);
    } else {
        return e.type;
    }
}
Type TypeCheck::visit(CallExpressionNode& e) {
    const std::string& name = table_.findStringByIdx(e.value);
    if (isRuntimeFunc(name)) {
        const RuntimeFunc& rFunc = getRuntimeFunc(name);
        if (rFunc.args.size() != e.operands.size()) {
            std::cerr << "TypeCheck Error: expected" << rFunc.args.size() << " operands but got " << e.operands.size() << " for function " << name << "!\n";
            this->errors++;
            return rFunc.retType;
        }
        for (std::size_t i = 0; i < e.operands.size(); i++) {
            Type expected = rFunc.args.at(i);
            Type got = e.operands.at(i)->accept(*this);
            if(!conforms(expected, got)) {
                std::cerr << "TypeCheck Error: expected operand " << i+1 << " type " << typeName(expected) << " but got type " << typeName(got) << "!\n";
            errors++;
            }
        }
        return rFunc.retType;
    }


    const std::vector<Type>& ops = funcOpCount[e.value];
    if (ops.size() != e.operands.size()) {
        std::cerr << "TypeCheck Error: expected " << ops.size() << " operands but got " << e.operands.size() << "!\n";
        errors++;
        return funcType[e.value];
    }
    for (std::size_t i = 0; i < e.operands.size(); i++) {
        Type expected = ops.at(i);
        Type got = e.operands.at(i)->accept(*this);
        if(!conforms(expected, got)) {
            std::cerr << "TypeCheck Error: expected operand " << i+1 << " type " << typeName(expected) << " but got type " << typeName(got) << "!\n";
        errors++;
        }
    }
    return funcType[e.value];
}

Type TypeCheck::visit(IndexExpressionNode& e) {
    Type t = e.primaryExpr->accept(*this);
    e.primaryExpr->type = t;
    Type idx = e.indexExpr->accept(*this);
    if (idx.type != TypeKind::i32 && idx.element != nullptr) {
        std::cerr << "TypeCheck Error: index needs to be of type integer" << std::endl;
        errors++;
    }
    if (t.type != TypeKind::Array) {
        std::cerr << "TypeCheck Error: cannot index a non-array type!\n";
    }
    Type result = *t.element;
    e.inferredType = result;
    return result;
}

Type TypeCheck::visit(CastExpressionNode& e) {
    Type resultingType = e.accept(*this);
    ExpressionNode* raw = e.expr.get();
    if (auto* termExpr = dynamic_cast<TermExpressionNode*>(raw)) {
        if (termExpr->isIdent) {
            
            if (e.type.type == TypeKind::Array) {
                std::cout << "TypeCheck Error: cannot cast a const array type to a mutable array type!\n";
                this->errors++;
            }
        }
    }
    return e.type;
}

Type TypeCheck::getVarType(int idx) {
    int scope = currentScope;
    Type type = Type::Void();
    while (scope >= 0) {
        if (varType.find(scope) != varType.end()) {
            if (varType[scope].find(idx) != varType[scope].end()) {
                type = varType[scope][idx];
            }
        }
        scope--;
    }
    return type;
}

bool TypeCheck::conforms(Type value, Type expected) {
    if (value.type == TypeKind::i64 && expected.type == TypeKind::u8)
        return true;
    if (value.type == TypeKind::u8 && expected.type == TypeKind::i32)
        return true;
    return value.type == expected.type;
}
