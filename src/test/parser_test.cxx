#include <iostream>
#include <string>

#include "str_utils.hxx"
#include "parser_test.hxx"
#include "test_utils.hxx"
#include "parser.hxx"

int check (std::unique_ptr<bassoon::ExprAST> result){
    if(!result){
        fprintf(stderr,"FAILED\n");
        return 1;
    }
    fprintf(stderr,"pass");
    return 0;
}

namespace bassoon{
namespace test{


int test_bool_expr(){
    fprintf(stderr,"test_bool_expr\n");
    utils::setup_input_string("true");
    std::unique_ptr<ExprAST> result = Parser::_testParseExpression();
    return check(std::move(result));
}
int test_int_expr();
int test_double_expr();

int test_paren_expr();

int test_binop_expr();

int test_call_expr();

    
int test_parser(){
    utils::setupSource();
    test_bool_expr();
    return 0;
}

} // namespace test
} // namespace bassoon
