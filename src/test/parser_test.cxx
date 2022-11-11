#include <iostream>
#include <string>

#include "str_utils.hxx"
#include "parser_test.hxx"
#include "test_utils.hxx"
#include "parser.hxx"

namespace bassoon{
namespace test{
    
int test_parser(){
    utils::setup_input_string("(a+a)");
    utils::setupSource();
    Parser::_testParseExpression();
    return 0;
}

} // namespace test
} // namespace bassoon
