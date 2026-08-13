#include "lexer.h"
#include "parser.h"
#include "runtime.h"
#include "token.h"
#include <iostream>
#include <fstream>
#include "printer.h"
#include "cgen.h"
#include "scopecheck.h"
#include "typecheck.h"

int main(int argc, const char** argv) {
    for (int i = 0; i < argc; i++) {
        std::cout << argv[i] << std::endl;
    }
    const char* filepath = argv[1];
    Lexer lexer(filepath);
    std::vector<Token> tokens = lexer.tokenise();
    std::cout << "Parsing...\n";
    Parser parser(tokens, lexer.stringtable_);
    ProgramNode program = parser.parse();
    
    std::cout << "Printing...\n";
    Printer printer;
    printer.print(program);

    std::cout << "Hooking Runtime...\n";
    Runtime::implement(program);
    
    std::cout << "ScopeChecking...\n";
    ScopeCheck scopeCheck(lexer.stringtable_);
    scopeCheck.visit(program);
    if (scopeCheck.errors) {
        std::cout << "Got " << scopeCheck.errors << " errors from ScopeCheck! Aborting...\n";
        exit(EXIT_FAILURE);
    }
    
    std::cout << "TypeChecking ...\n";
    TypeCheck typeCheck(lexer.stringtable_);
    typeCheck.visit(program);
    if (typeCheck.errors) {
        std::cout << "Got " << typeCheck.errors << " errors from TypeCheck! Aborting...\n";
        exit(EXIT_FAILURE);
    }
    
    std::cout << "Code generation...\n";
    Cgen cgen(lexer.stringtable_);
    std::string output = cgen.generate(program);

    std::ofstream outputFile("output.s");
    outputFile << output;
    outputFile.close();

    std::cout << "Done!...\n"; 
    
    return 0;
}
