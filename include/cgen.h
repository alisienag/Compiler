#pragma once
#include "ast.h"
#include "stringtable.h"
#include <sstream>
#include <string>
#include <unordered_map>

#define R_RBP "rbp"
#define R_RSP "rsp"

#define R_EAX "eax"
#define R_EBX "ebx"

#define R_RAX "rax"
#define R_RBX "rbx"
#define R_RDI "rdi"
#define R_RSI "rsi"
#define R_RDX "rdx"

#define SYS_EXIT "60"

#define F_DONE "_done"
class Cgen : public Visitor {
    public:
        Cgen(StringTable table);
        std::string generate(ProgramNode& program);

        void visit(ProgramNode&) override;
        void visit(FunctionNode&) override;
        void visit(OperandNode&) override;

        void visit(LetStatementNode&) override;
        void visit(BlockStatementNode&) override;
        void visit(ReassignStatementNode&) override;
        void visit(ReturnStatementNode&) override;

        void visit(ExpressionStatementNode&) override;

        void visit(BinaryExpressionNode&) override;
        void visit(TermExpressionNode&) override;
        void visit(CallExpressionNode&) override;
    private:
        std::stringstream assembly_;
        std::stringstream data;
        StringTable table_;
        std::unordered_map<int, int> localIndex_;
        std::unordered_map<int, int> operandIndex_;
        int firstIndex = 0;
        
        std::string currentFunction;
        bool hasReturn = false;

        std::string getLocalLocation(int idx);
        std::string getOperandLocation(int idx);
        void emitLine(const std::string& s) { assembly_ << "    " << s << "\n"; }
        void emitLabel(const std::string& s) { assembly_ << s << ":\n"; }
        
        void emitPush(int imm) { emitLine("push " + std::to_string(imm)); }
        void emitPush(const std::string& r) { emitLine("push " + r); }
        void emitPop(const std::string& r) { emitLine("pop " + r); }

        void emitMov(const std::string& dst, const std::string& src) {
            emitLine("mov " + dst + ", " + src);
        }

        void emitCall(int idx) {
            emitLine("call " + table_.findStringByIdx(idx));
        }
        
        void emitSyscall() {
            emitLine("syscall");
        }
        
        void emitPrint();
};
