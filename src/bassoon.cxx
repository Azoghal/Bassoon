#include "lexer.hxx"
#include "parser.hxx"
#include "tokens.hxx"
#include "viz_visitor.hxx"
#include "type_visitor.hxx"
#include "codegen.hxx"
#include <iostream>

#include "spdlog/spdlog.h"

int main(int argc, char *argv[]){
    spdlog::set_level(spdlog::level::info);
    spdlog::info("Starting Bassoon");

    std::shared_ptr<bassoon::BProgram> program = bassoon::Parser::parseLoop();

    // Either switch back to manual delete to close files after visualisation
    // or close them earlier than destruction
    bassoon::viz::VizVisitor * p_visualiser = new bassoon::viz::VizVisitor("Parse");
    p_visualiser->visualiseAST(program);
    delete p_visualiser;
 
    bassoon::typecheck::TypeVisitor typechecker;
    typechecker.typecheck(program);

    bassoon::viz::VizVisitor * tc_visualiser = new bassoon::viz::VizVisitor("Typecheck");
    tc_visualiser->visualiseAST(program);
    delete tc_visualiser;

    bassoon::codegen::CodeGenerator code_generator;
    code_generator.definePutChar();
    code_generator.generate(program);
    //code_generator.printIR();
    code_generator.optimize();
    code_generator.setTarget();
    code_generator.compile();

    return 0;
}
