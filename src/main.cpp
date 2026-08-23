#include "codegen.h"
#include "codegenemitter.h"
#include "lexer.h"
#include "parser.h"
#include "runtime.h"
#include "scopecheck.h"
#include "token.h"
#include "typecheck.h"
#include <iostream>
#include <fstream>

int main(int argc, const char** argv) {
    for (int i = 0; i < argc; i++) {
        std::cout << argv[i] << std::endl;
    }
    if (argc != 1 ) {
        std::cout << "This program takes one argument, being the source file!\n";
    }
    const char* filepath = argv[1];
    Lexer lexer(filepath);
    std::vector<Token> tokens = lexer.tokenise();
    Diagnostics diag;

    std::cout << "Parsing...\n";
    Parser parser(tokens, lexer.stringtable_, diag);
    std::unique_ptr<Program> p = parser.parseProgram();
    diag.listErrors();
    diag.clearErrors();
    std::cout << "Parsing done!\n";
    
    Runtime::install(*p, lexer.stringtable_);

    SymbolArena arena;

    ScopeCheck scopeCheck(lexer.stringtable_, arena, diag);
    std::cout << "ScopeChecker running...\n";
    scopeCheck.visit(*p);
    if (diag.hasErrors()) {
        diag.listErrors();
        return EXIT_FAILURE;
    }
    std::cout << "ScopeChecker done!\n";

    TypeCheck typeCheck(lexer.stringtable_, arena, diag);
    std::cout << "TypeChecker running...\n";
    typeCheck.visit(*p);
    if (diag.hasErrors()) {
        diag.listErrors();
        return EXIT_FAILURE;
    }
    std::cout << "TypeChecker done!\n";

    CodeGenEmitter emitter;
    CodeGen codeGen(lexer.stringtable_, arena, diag, emitter);
    std::cout << "CodeGen running...\n";
    codeGen.visit(*p);
    std::cout << "CodeGen done!\n";
    
    std::ofstream outputFile("output.s");
    outputFile << emitter.getOutput();
    outputFile.close();

    std::cout << "Done!...\n"; 
    
    return 0;
}
