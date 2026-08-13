#pragma once
#include "ast.h"
#include "stringtable.h"
#include <sstream>
#include <string>
#include <unordered_map>

#define R_RBP "rbp"
#define R_RSP "rsp"

#define R_RAX "rax" //64 bit
#define R_EAX "eax" //32 bit
#define R_AX "ax" //16 bit
#define R_AL "al" //8 bit

#define R_RBX "rbx"
#define R_EBX "ebx"

#define R_RCX "rcx"

#define R_RDI "rdi"
#define R_RSI "rsi"
#define R_RDX "rdx"

#define SYS_EXIT "60"

#define F_DONE "_done"
class Cgen : public Visitor {
    public:
        Cgen(StringTable& table) : table_(table) {}
        std::string generate(ProgramNode& program);

        Type visit(ProgramNode&) override;
        Type visit(FunctionNode&) override;
        Type visit(OperandNode&) override;

        Type visit(LetStatementNode&) override;
        Type visit(BlockStatementNode&) override;
        Type visit(ReassignStatementNode&) override;
        Type visit(ReturnStatementNode&) override;
        Type visit(IfStatementNode&) override;

        Type visit(ExpressionStatementNode&) override;

        Type visit(BinaryExpressionNode&) override;
        Type visit(TermExpressionNode&) override;
        Type visit(CallExpressionNode&) override;
        Type visit(CastExpressionNode&) override;
        Type visit(IndexExpressionNode&) override;
    private:
        std::stringstream assembly_;
        std::stringstream data;

        StringTable& table_;

        std::unordered_map<int, std::unordered_map<int, int>> localIndex_;
        std::unordered_map<int, std::unordered_map<int, bool>> isConst_;
        int currentScope = 0;

        std::unordered_map<int, int> operandIndex_;
        int firstIndex = 0;
        int stackDepth = 0;

        std::vector<int> stringsEmitted;
        
        std::string currentFunction;
        bool hasReturn = false;

        std::string getLocalLocation(int idx);
        std::string getOperandLocation(int idx);
        void emitLine(const std::string& s) { assembly_ << "    " << s << "\n"; }
        void emitLabel(const std::string& s) { assembly_ << s << ":\n"; }

        int labels_ = 0;
        std::string getFreshLabel() { return "label_" + std::to_string(labels_++); }
        
        void emitJmp(const std::string& label) { emitLine("jmp " + label); }
        void emitJe(const std::string& label) { emitLine("je " + label); }

        void emitCmp(const std::string& l, const std::string& r) {
            emitLine("cmp " + l + ", " + r);
        }
        
        void emitPush(int imm) { emitLine("push " + std::to_string(imm)); stackDepth++; }
        void emitPush(const std::string& r) { emitLine("push " + r); stackDepth++; }
        void emitPop(const std::string& r) { emitLine("pop " + r); stackDepth--; }

        void emitMov(const std::string& dst, const std::string& src) {
            emitLine("mov " + dst + ", " + src);
        }

        void emitCall(int idx) {
            emitLine("call _" + table_.findStringByIdx(idx));
        }
        
        void emitSyscall() {
            emitLine("syscall");
        }
        
        void emitPrint();
};
