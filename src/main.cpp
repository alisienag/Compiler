#include "lexer.h"
#include "parser.h"
#include "token.h"
#include <iostream>
#include <fstream>
#include "printer.h"
#include "cgen.h"
#include "scopecheck.h"

int main(int argc, const char** argv) {
    for (int i = 0; i < argc; i++) {
        std::cout << argv[i] << std::endl;
    }
    const char* filepath = argv[1];
    Lexer lexer(filepath);
    std::vector<Token> tokens = lexer.tokenise();
    for (std::size_t i = 0; i < tokens.size(); i++) {
        Token token = tokens.at(i);
        std::cout << "TokenType: ";
        if (token.type == TokenType::Let) {
            std::cout << "Let";
        } else if (token.type == TokenType::i32) {
            std::cout << "i32";
        } else if (token.type == TokenType::Colon){
            std::cout << ":";
        } else if (token.type == TokenType::Equals){
            std::cout << "=";
        } else if (token.type == TokenType::SColon){
            std::cout << ";";
        } else if (token.type == TokenType::Identifier) {
            std::cout << "Identifier: " << lexer.stringtable_.findStringByIdx(token.value);
        } else if (token.type == TokenType::Error){
            std::cout << "ERROR";
        } else {
            std::cout << (int)token.type;
        }
        std::cout << " value: " << token.value << std::endl;
    }
    std::cout << "Parsing...\n";
    Parser parser(tokens, lexer.stringtable_);
    ProgramNode program = parser.parse();
    
    std::cout << "Printing...\n";
    Printer printer;
    printer.print(program);
    
    std::cout << "ScopeChecking...\n";
    ScopeCheck scopeCheck(lexer.stringtable_);
    scopeCheck.visit(program);
    
    std::cout << "Code generation...\n";
    Cgen cgen(lexer.stringtable_);
    std::string output = cgen.generate(program);

    std::ofstream outputFile("output.s");
    outputFile << output;
    outputFile.close();

    std::cout << "Done!...\n"; 
    
    return 0;
}
