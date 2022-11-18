#include "lexer.hxx"
#include "parser.hxx"
#include "tokens.hxx"
#include "astviz.hxx"
#include <iostream>

int main(int argc, char *argv[]){
    bassoon::Parser::setVerbosity(1);
    std::vector<std::unique_ptr<bassoon::NodeAST>> ASTs;
    bassoon::Parser::parseLoop(std::move(ASTs));
    
    return 0;
}
