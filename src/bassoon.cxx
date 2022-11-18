#include "lexer.hxx"
#include "parser.hxx"
#include "tokens.hxx"
#include "viz_visitor.hxx"
#include <iostream>

int main(int argc, char *argv[]){
    bassoon::Parser::setVerbosity(1);

    std::vector<std::unique_ptr<bassoon::NodeAST>> ASTs;
    bassoon::Parser::parseLoop(std::move(ASTs));
    fprintf(stderr,"Parsed successfully\n");

    bassoon::viz::VizVisitor visualiser;
    fprintf(stderr,"Made a visualiser\n");

    fprintf(stderr,"size %i\n", ASTs.size());

    std::unique_ptr<bassoon::NodeAST> node = std::move(ASTs.at(ASTs.size()-1));
    fprintf(stderr,"node successfully\n");

    ASTs.pop_back();
    fprintf(stderr,"popped successfully\n");

    visualiser.visualiseAST(std::move(ASTs[ASTs.size()-1]));
    fprintf(stderr,"ran visualiseAST successfully\n");

    ASTs.pop_back();
    fprintf(stderr,"Popped successfully\n");
    return 0;
}
