#include "lexer.hxx"
#include "parser.hxx"
#include "tokens.hxx"
#include "viz_visitor.hxx"
#include <iostream>

int main(int argc, char *argv[]){
    bassoon::Parser::setVerbosity(1);
    std::vector<std::unique_ptr<bassoon::NodeAST>> ASTs;
    bassoon::Parser::parseLoop(std::move(ASTs));
    bassoon::viz::VizVisitor * visualiser = new bassoon::viz::VizVisitor;
    visualiser->visualiseAST(std::move(ASTs[ASTs.size()-1]));
    ASTs.pop_back();
    return 0;
}
