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
    fprintf(stderr,"pass\n");
    return 0;
}

namespace bassoon{
namespace test{

int countParserTestFails(std::vector<int> source_tokens){
    int failures = 0;
    utils::setupParseSourceTokens(source_tokens);
    std::unique_ptr<ExprAST> result = Parser::_testParseExpression();
    failures += check(std::move(result));
    return failures;
}


int test_bool_expr(){
    fprintf(stderr,"test_bool_expr\n");
    int failures = 0;
    std::vector<int> source_tokens = {tok_false, tok_eof};

    failures += countParserTestFails(source_tokens);

    source_tokens = {tok_true, tok_eof};

    failures += countParserTestFails(source_tokens);
    return failures;
}

int test_int_expr(){
    fprintf(stderr,"test_int_expr\n");
    std::vector<int> source_tokens = {tok_number_int, tok_eof};
    return countParserTestFails(source_tokens);
}

int test_double_expr(){
    fprintf(stderr,"test_double_expr\n");
    std::vector<int> source_tokens = {tok_number_double, tok_eof};
    return countParserTestFails(source_tokens);
}

int test_binop_expr(){
    fprintf(stderr,"test_binop_expr\n");
    int failures = 0;
    std::vector<int> source_tokens = {tok_number_int,'+',tok_number_int, tok_eof};

    failures += countParserTestFails(source_tokens);

    source_tokens = {tok_number_int,'+',tok_number_int,'+',tok_number_int, tok_eof};

    failures += countParserTestFails(source_tokens);
    return failures;
}

int test_paren_expr(){
    fprintf(stderr,"test_paren_expr\n");
    int failures = 0;
    std::vector<int> source_tokens = {'(',tok_number_int,')', tok_eof};

    failures += countParserTestFails(source_tokens);

    source_tokens = {'(',tok_number_int,'+',tok_number_int,')', tok_eof};

    failures += countParserTestFails(source_tokens);
    return failures;
}

int test_call_expr(){
    fprintf(stderr,"test_call_expr\n");
    int failures = 0;
    std::vector<int> source_tokens = {tok_identifier,'(',')', tok_eof};

    failures += countParserTestFails(source_tokens);

    source_tokens = {tok_identifier,'(',tok_number_int,')', tok_eof};

    failures += countParserTestFails(source_tokens);

    source_tokens = {tok_identifier,'(',tok_identifier, ',', tok_identifier, ')', tok_eof};

    failures += countParserTestFails(source_tokens);
    return failures;
}
    
int test_expressions(){
    test_bool_expr();
    test_int_expr();
    test_double_expr();
    test_binop_expr();
    test_paren_expr();
    test_call_expr();
    return 0;
}

int test_parser(){
    utils::setupParserSource();
    test_expressions();
    return 0;
}

} // namespace test
} // namespace bassoon
