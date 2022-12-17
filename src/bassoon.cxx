#include "lexer.hxx"
#include "parser.hxx"
#include "tokens.hxx"
#include "viz_visitor.hxx"
#include "type_visitor.hxx"
#include <iostream>

int main(int argc, char *argv[]){
    bassoon::Parser::setVerbosity(1);

    std::vector<std::unique_ptr<bassoon::NodeAST>>  * ASTs = new std::vector<std::unique_ptr<bassoon::NodeAST>>();
    auto statements_and_definitions = bassoon::Parser::parseLoop();
    auto statement_ASTs = std::move(statements_and_definitions.first);
    auto definition_ASTs = std::move(statements_and_definitions.second);

    std::shared_ptr<bassoon::NodeAST> node;
    // if(statement_ASTs.size() == 0){
    //     fprintf(stderr, "no parsed top level statements ASTs\n");
    //     return 1;
    // }
    // node = std::move(statement_ASTs.at(statement_ASTs.size()-1));
    // statement_ASTs.pop_back();

    if(definition_ASTs.size() == 0){
        fprintf(stderr, "no parsed definition ASTs\n");
        return 1;
    }
    node = std::move(definition_ASTs.at(definition_ASTs.size()-1));
    statement_ASTs.pop_back();

    // bassoon::viz::VizVisitor * visualiser = new bassoon::viz::VizVisitor("Parse");
    // visualiser->visualiseAST(node);
    // delete visualiser;

    bassoon::typecheck::TypeVisitor typechecker;
    typechecker.typecheckAST(node);

    // visualiser = new bassoon::viz::VizVisitor("Typecheck");
    // visualiser->visualiseAST(node);
    // delete visualiser;

    fprintf(stderr, "Typecheck completed\n");
    return 0;
}
