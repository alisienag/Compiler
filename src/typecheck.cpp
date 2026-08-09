#include "typecheck.h"
#include "ast.h"
#include <iostream>

Type TypeCheck::visit(ProgramNode& p) {
    errors = 0;
    for (auto& funcs : p.functions) {
        funcs->accept(*this);
    }
    return Type::Unknown;
}
Type TypeCheck::visit(FunctionNode& f) {
    funcType[f.nameIdx] = f.type;
    currentScope++;
    for (auto& op : f.operands)
        funcOpCount[f.nameIdx].push_back(op->accept(*this));
    Type stmtType = f.statement->accept(*this);
    if (!conforms(f.type, stmtType)) {
        std::cerr << "TypeCheck Error: function << " << table_.findStringByIdx(f.nameIdx) << " of return type " << typeName(f.type) << " cannot return type of " << typeName(stmtType) << "\n";
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
    return Type::Unknown;
}
Type TypeCheck::visit(BlockStatementNode& s) {
    Type type = Type::Unknown;
    currentScope++;
    for (auto& stmt : s.statements) {
        Type stmtType = stmt->accept(*this);
        if (stmtType != Type::Unknown) { // all statements return Unknown except for the return statement!
            type = stmtType;
        }
    }
    currentScope--;
    return type;
}

Type TypeCheck::visit(ReassignStatementNode& s) {
    Type exprType = s.expr->accept(*this);
    if (!conforms(getVarType(s.idx), exprType)) {
        std::cerr << "TypeCheck Error: cannot reassign type " << typeName(exprType) << " to variable of type " << typeName(getVarType(s.idx));
        errors++;
    }
    return Type::Unknown;
}

Type TypeCheck::visit(ReturnStatementNode& s) {
    return s.expr->accept(*this);
}
Type TypeCheck::visit(ExpressionStatementNode& s) {
    s.expr->accept(*this);
    return Type::Unknown;
}

Type TypeCheck::visit(IfStatementNode& s) {
    Type condType = s.cond->accept(*this);
    if (condType != Type::Bool) {
        std::cerr << "TypeCheck Error: expected an expression of type bool for if\n";
        errors++;
    }
    Type ifType = s.ifNode->accept(*this);
    Type elseType = Type::Unknown;
    if (s.hasElse) {
        elseType = s.elseNode->accept(*this);
        if (ifType != elseType) {
            std::cout << "TypeCheck warning: if and else branches dont return the same type!\n";
        }
    }
    return ifType;
}

Type TypeCheck::visit(BinaryExpressionNode& e) {
    Type lType = e.l->accept(*this);
    Type rType = e.r->accept(*this);
    if (lType != rType) {
        std::cerr << "TypeCheck Error: cannot perform " << opName(e.op) << " operation on differing types!\n";
        errors++;
    }
    if (e.op == BinOp::Eq) {
        e.inferredType = Type::Bool;
        return Type::Bool;
    }
    if (lType == Type::i32) {
        e.inferredType = Type::i32;
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
    const std::vector<Type>& ops = funcOpCount[e.value];
    if (ops.size() != e.operands.size()) {
        std::cerr << "TypeCheck Error: expected " << ops.size() << " operands but got " << e.operands.size() << "!\n";
        errors++;
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

Type TypeCheck::getVarType(int idx) {
    int scope = currentScope;
    Type type = Type::Unknown;
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
    return value == expected;
}
