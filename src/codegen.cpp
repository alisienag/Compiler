#include "codegen.h"
#include "ast.h"
#include "codegenemitter.h"

#define REG_RBP emitter_.getReg(CodeGenReg::RBP)
#define REG_RSP emitter_.getReg(CodeGenReg::RSP)

#define REG_RAX emitter_.getReg(CodeGenReg::RAX)
#define REG_RBX emitter_.getReg(CodeGenReg::RBX)
#define REG_RCX emitter_.getReg(CodeGenReg::RCX)
#define REG_RDX emitter_.getReg(CodeGenReg::RDX)

void CodeGen::visit(Program& p) {
    emitter_.setContext(TEXT_CONTEXT);
    emitter_.emitLabel("_start");
    emitter_.emitCall("start");
    emitter_.emitMov("rdi", "rax");
    emitter_.emitMov("rax", "60");
    emitter_.emitLine("syscall");

    emitter_.setContext(TEXT_CONTEXT);
    for (auto& funcs : p.functions) {
        currentFn_ = funcs.get();
        funcs->accept(*this);
        currentFn_ = nullptr;
    }
}

void CodeGen::visit(FunctionDecl& f) {
    if (f.isExtern()) return;
    emitter_.emitLabel(f.asmName);
    emitter_.emitPush(REG_RBP);
    emitter_.emitMov(REG_RBP, REG_RSP);
    unsigned int frameSize = (f.frameSize + 15) & ~15U;
    if (frameSize)
        emitter_.emitSub(REG_RSP, std::to_string(frameSize));
    f.body->accept(*this);
    emitter_.emitLabel(f.asmName + "_end");
    emitter_.emitMov(REG_RSP, REG_RBP);
    emitter_.emitPop(REG_RBP);
    emitter_.emitRet();
}

void CodeGen::visit(Param& p) {
}

void CodeGen::visit(LetStmt& s) {
    s.init->accept(*this);
    if (s.symbol) {
        int offset = s.symbol->offset;
        emitter_.emitMov(emitter_.getMem(REG_RBP, offset), REG_RAX);
    }
}

void CodeGen::visit(ExprStmt& s) {
    if (!s.target) {
        s.value->accept(*this);
        return;
    }
    emitAddress(*s.target);
    emitter_.emitPush(REG_RAX);
    s.value->accept(*this);
    emitter_.emitPop(REG_RCX);
    unsigned int width = dynamic_cast<IndexExpr*>(s.target.get())
        ? sizeOf(s.target->type) : 8;
    emitStore(REG_RCX, width);
}

void CodeGen::visit(ReturnStmt& s) {
    if (s.value)
        s.value->accept(*this);
    emitter_.emitJmp(currentFn_->asmName + "_end");
}

void CodeGen::visit(IfStmt& s) {
    unsigned int elseBranch = emitter_.getLabel();
    unsigned int doneBranch = emitter_.getLabel();

    s.cond->accept(*this);
    emitter_.emitLine("cmp rax, 0");
    emitter_.emitJe(s.hasElse() ? elseBranch : doneBranch);
    s.thenBlock->accept(*this);
    if (s.hasElse()) {
        emitter_.emitJmp(doneBranch);
        emitter_.emitLabel(elseBranch);
        s.elseBranch->accept(*this);
    }
    emitter_.emitLabel(doneBranch);
}

void CodeGen::visit(WhileStmt& s) {
    unsigned int startWhile = emitter_.getLabel();
    unsigned int endWhile = emitter_.getLabel();
    unsigned int prevContinueLabel = continueLabel_;
    unsigned int prevBreakLabel = breakLabel_;
    continueLabel_ = startWhile;
    breakLabel_ = endWhile;

    emitter_.emitLabel(startWhile);

    s.cond->accept(*this);
    emitter_.emitLine("cmp rax, 0");
    emitter_.emitJe(endWhile);

    s.body->accept(*this);
    emitter_.emitJmp(startWhile);
    emitter_.emitLabel(endWhile);

    continueLabel_ = prevContinueLabel;
    breakLabel_ = prevBreakLabel;
}

void CodeGen::visit(BreakStmt&) {
    emitter_.emitJmp(breakLabel_);
}

void CodeGen::visit(ContinueStmt&) {
    emitter_.emitJmp(continueLabel_);
}

void CodeGen::visit(BlockStmt& s) {
    for (auto& stmt : s.statements)
        stmt->accept(*this);
}


void CodeGen::visit(CallExpr& e) {
    size_t n = e.args.size();
    bool pad = (n % 2) != 0;
    if (pad) {
        emitter_.emitPush("0");
    }
    for (auto it = e.args.rbegin(); it != e.args.rend(); ++it) {
        it->get()->accept(*this);
        emitter_.emitPush(REG_RAX);
    }
    NameExpr* nameExpr = dynamic_cast<NameExpr*>(e.callee.get());
    if (nameExpr && nameExpr->symbol) {
        emitter_.emitCall(nameExpr->symbol->asmName);
    } else {
        diags_.error(e.span, "Error finding symbol for call expression!");
    }
    if (n || pad) {
        emitter_.emitAdd(REG_RSP, std::to_string(8 * n + (pad ? 8 : 0)));
    }
}

void CodeGen::visit(IndexExpr& e) {
    emitAddress(e);
    emitLoad(REG_RAX, e.type);
}

void CodeGen::visit(CastExpr& e) {
    e.operand->accept(*this);

    const Type& from = e.operand->type;
    const Type& to = e.target;
    
    if (from.isArray() || to.isArray()) return;
    if (from == to) return;

    if (to.kind == TypeKind::Bool) {
        emitter_.emitLine("cmp rax, 0");
        emitter_.emitLine("setne al");
        emitter_.emitLine("movzx rax, al");
        return;
    }

    if (from.kind == TypeKind::Bool) return;

    unsigned int fromSize = sizeOf(from);
    unsigned int toSize = sizeOf(to);

    if (toSize <= fromSize) {
        switch(toSize) {
            case 1: emitter_.emitLine("movzx rax, al"); break;
            case 2: emitter_.emitLine("movzx rax, ax"); break;
            case 4: emitter_.emitLine("mov eax, eax"); break;
            default: break;
        }
        return;
    }

    switch(fromSize) {
        case 1: emitter_.emitLine(from.isSigned() ? "movsx rax, al" : "movzx rax, al"); break;
        case 2: emitter_.emitLine(from.isSigned() ? "movsx rax, ax" : "movzx rax, ax"); break;
        case 4: emitter_.emitLine(from.isSigned() ? "movsxd rax, eax" : "mov eax, eax"); break;
    }
}

void CodeGen::visit(LogicalExpr& e) {
    unsigned int trueLabel = emitter_.getLabel();
    unsigned int falseLabel = emitter_.getLabel();
    unsigned int endLabel = emitter_.getLabel();
    e.lhs->accept(*this);
    emitter_.emitCmp(REG_RAX, std::to_string(0));
    if (e.op == LogicalOp::Or) {
        emitter_.emitJne(trueLabel);
    } else if (e.op == LogicalOp::And) {
        emitter_.emitJe(falseLabel);
    }
    e.rhs->accept(*this);
    emitter_.emitCmp(REG_RAX, std::to_string(0));
    emitter_.emitJne(trueLabel);

    emitter_.emitLabel(falseLabel);
    emitter_.emitMov(REG_RAX, std::to_string(0));
    emitter_.emitJmp(endLabel);

    emitter_.emitLabel(trueLabel);
    emitter_.emitMov(REG_RAX, std::to_string(1));

    emitter_.emitLabel(endLabel);
}

void CodeGen::visit(BinaryExpr& e) {
    e.rhs->accept(*this);
    emitter_.emitPush(REG_RAX);
    e.lhs->accept(*this);
    emitter_.emitPop(REG_RBX);
    switch (e.op) {
        case BinOp::Add:
            emitter_.emitAdd(REG_RAX, REG_RBX); return;
        case BinOp::Sub:
            emitter_.emitSub(REG_RAX, REG_RBX); return;
        case BinOp::Mul:
            emitter_.emitIMul(REG_RAX, REG_RBX); return;
        case BinOp::Div:
            if (e.type.isSigned()) {
                emitter_.emitLine("cqo");
                emitter_.emitLine("idiv rbx");
            } else {
                emitter_.emitMov(REG_RDX, "0");
                emitter_.emitDiv(REG_RBX);
            }
            return;
        case BinOp::Eq:
        case BinOp::Ne:
        case BinOp::Ge:
        case BinOp::Gt:
        case BinOp::Le:
        case BinOp::Lt:
        {
            emitter_.emitCmp(REG_RAX, REG_RBX);
            const char* cc;
            bool sign = e.lhs->type.isSigned();
            switch(e.op) {
                default:
                case BinOp::Eq: cc = "sete"; break;
                case BinOp::Ne: cc = "setne"; break;
                case BinOp::Lt: cc = sign ? "setl" : "setb"; break;
                case BinOp::Gt: cc = sign ? "setg" : "seta"; break;
                case BinOp::Le: cc = sign ? "setle" : "setbe"; break;
                case BinOp::Ge: cc = sign ? "setge" : "setae"; break;
            }
            emitter_.emitLine(std::string(cc) + " al");
            emitter_.emitLine("movzx rax, al");
            return;
        }
    }
    return;
}

void CodeGen::visit(UnaryExpr& e) {
    e.operand->accept(*this);
    if (e.op == UnaryOp::Neg) {
        emitter_.emitLine("neg rax");
    } else {
        emitter_.emitLine("cmp rax, 0");
        emitter_.emitLine("sete al");
        emitter_.emitLine("movzx rax, al");
    }
}

void CodeGen::visit(NameExpr& e) {
    if (e.symbol) {
        int offset = e.symbol->offset;
        emitter_.emitMov(REG_RAX, emitter_.getMem(REG_RBP, offset));
    }
}

void CodeGen::visit(StringLiteral& e) {
    if (e.label == -1) {
        e.label = strLabels_++;
        emitter_.setContext(DATA_CONTEXT);
        emitter_.emitLabel("string_" + std::to_string(e.label));
        emitter_.emit(".ascii \"");
        emitter_.emit(e.value);
        emitter_.emit("\"\n");
        emitter_.setContext(TEXT_CONTEXT);
    }
    emitter_.emitLea(REG_RAX, "string_" + std::to_string(e.label));
}

void CodeGen::visit(BoolLiteral& e) {
    emitter_.emitMov(REG_RAX, std::to_string(e.value));
}

void CodeGen::visit(IntLiteral& e) {
    emitter_.emitMov(REG_RAX, std::to_string(e.value));
}

void CodeGen::emitStore(const std::string& reg, unsigned int width) {
    switch (width) {
        case 1:
            emitter_.emitLine("mov byte ptr [" + reg + "], al");
            return;
        case 2:
            emitter_.emitLine("mov word ptr [" + reg + "], ax");
            return;
        case 4:
            emitter_.emitLine("mov dword ptr [" + reg + "], eax");
            return;
        default:
            emitter_.emitLine("mov [" + reg + "], rax");
            return;
    }
}

void CodeGen::emitLoad(const std::string& reg, const Type& t) {
    const std::string m = emitter_.getMem(reg, 0);
    switch (sizeOf(t)) {
        case 1:
            emitter_.emitLine((t.isSigned() ?
                    "movsx rax, byte ptr " : "movzx rax, byte ptr ") + m);
            return;
        case 2:
            emitter_.emitLine((t.isSigned() ?
                    "movsx rax, word ptr " : "movzx rax, word ptr ") + m);
            return;
        case 4:
            emitter_.emitLine((t.isSigned() ?
                    "movsxd rax, dword ptr " : "mov eax, dword ptr ") + m);
            return;
        default:
            emitter_.emitLine("mov rax, " + m);
            return;
    }
}

void CodeGen::emitAddress(Expression& e) {
    if (auto* i = dynamic_cast<NameExpr*>(&e)) {
        emitter_.emitLea(REG_RAX, emitter_.getMem(REG_RBP, i->symbol->offset));
        return;
    } else if (auto* idx = dynamic_cast<IndexExpr*>(&e)) {
        idx->base->accept(*this);
        emitter_.emitPush(REG_RAX);
        idx->index->accept(*this);
        emitter_.emitPush(REG_RAX);
        emitter_.emitPop(REG_RCX);
        emitter_.emitPop(REG_RAX);
        unsigned int stride = sizeOf(*idx->base->type.element);
        if (stride != 1) {
            emitter_.emitIMul(REG_RCX, std::to_string(stride));
        }
        emitter_.emitAdd(REG_RAX, REG_RCX);
        return;
    }
    diags_.error(e.span, "emit address not an l-value!?");
}
