#include "lexer.hxx"
#include "parser.hxx"
#include "tokens.hxx"
#include "viz_visitor.hxx"
#include "type_visitor.hxx"
#include <iostream>

int main(int argc, char *argv[]){
    bassoon::Parser::setVerbosity(1);

    std::vector<std::unique_ptr<bassoon::NodeAST>>  * ASTs = new std::vector<std::unique_ptr<bassoon::NodeAST>>();
    bassoon::Parser::parseLoop(ASTs);

    if(ASTs->size() == 0){
        fprintf(stderr, "no parsed ASTs\n");
        return 1;
    }
    std::shared_ptr<bassoon::NodeAST> node = std::move(ASTs->at(ASTs->size()-1));
    ASTs->pop_back();

    bassoon::viz::VizVisitor * visualiser = new bassoon::viz::VizVisitor();
    visualiser->visualiseAST(node);
    delete visualiser;

    bassoon::typecheck::TypeVisitor typechecker;
    typechecker.typecheckAST(node);

    fprintf(stderr, "Typecheck completed\n");
    return 0;
}
