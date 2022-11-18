#include "lexer.hxx"
#include "parser.hxx"
#include "tokens.hxx"
#include "viz_visitor.hxx"
#include <iostream>

int main(int argc, char *argv[]){
    bassoon::Parser::setVerbosity(1);

    std::vector<std::shared_ptr<bassoon::NodeAST>>  * ASTs = new std::vector<std::shared_ptr<bassoon::NodeAST>>();
    bassoon::Parser::parseLoop(ASTs);
    fprintf(stderr,"Parsed successfully\n");

    bassoon::viz::VizVisitor visualiser;
    fprintf(stderr,"Made a visualiser\n");

    fprintf(stderr,"size %lu\n", ASTs->size());

    if(ASTs->size() == 0){
        fprintf(stderr, "no parsed ASTs\n");
        return 1;
    }
    std::shared_ptr<bassoon::NodeAST> node = std::move(ASTs->at(ASTs->size()-1));
    fprintf(stderr,"node successfully\n");

    ASTs->pop_back();
    fprintf(stderr,"popped successfully\n");

    visualiser.visualiseAST(node);
    fprintf(stderr,"ran visualiseAST successfully\n");
    return 0;
}
