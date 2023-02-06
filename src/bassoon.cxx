#include "lexer.hxx"
#include "parser.hxx"
#include "tokens.hxx"
#include "viz_visitor.hxx"
#include "type_visitor.hxx"
#include "codegen.hxx"
#include <iostream>

int main(int argc, char *argv[]){
    bassoon::Parser::setVerbosity(1);

    std::shared_ptr<bassoon::BProgram> program = bassoon::Parser::parseLoop();

    bassoon::viz::VizVisitor p_visualiser("Parse");
    p_visualiser.visualiseAST(program);
 
    bassoon::typecheck::TypeVisitor typechecker;
    typechecker.typecheck(program);

    bassoon::viz::VizVisitor tc_visualiser("Typecheck");
    tc_visualiser.visualiseAST(program);

    bassoon::codegen::CodeGenerator code_generator;
    code_generator.DefinePutChar();
    code_generator.Generate(program);
    code_generator.PrintIR();
    code_generator.SetTarget();
    code_generator.Compile();
    return 0;
}
