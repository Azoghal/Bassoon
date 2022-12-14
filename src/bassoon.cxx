#include "lexer.hxx"
#include "parser.hxx"
#include "tokens.hxx"
#include "viz_visitor.hxx"
#include "type_visitor.hxx"
#include <iostream>

int main(int argc, char *argv[]){
    bassoon::Parser::setVerbosity(1);

    //std::vector<std::unique_ptr<bassoon::NodeAST>>  * ASTs = new std::vector<std::unique_ptr<bassoon::NodeAST>>();
    std::shared_ptr<bassoon::BProgram> program = bassoon::Parser::parseLoop();

    bassoon::viz::VizVisitor * visualiser = new bassoon::viz::VizVisitor("Parse");
    visualiser->visualiseAST(program);
    delete visualiser;
 
    bassoon::typecheck::TypeVisitor typechecker;
    typechecker.typecheck(program);

    visualiser = new bassoon::viz::VizVisitor("Typecheck");
    visualiser->visualiseAST(program);
    delete visualiser;

    return 0;
}
