#include <iostream>
#include <string>

#include "str_utils.hxx"
#include "parser_test.hxx"
#include "test_utils.hxx"
#include "parser.hxx"

// Find a nice way to do these
int check (std::unique_ptr<bassoon::ExprAST> result){
    if(!result){
        fprintf(stderr,"FAILED\n");
        return 1;
    }
    fprintf(stderr,"pass\n");
    return 0;
}

int check (int result){
    if(result){
        fprintf(stderr,"FAILED\n");
        return 1;
    }
    fprintf(stderr,"pass\n");
    return 0;
}

namespace bassoon{
namespace test{

int countParserExprTestFails(std::vector<int> source_tokens){
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

    failures += countParserExprTestFails(source_tokens);

    source_tokens = {tok_true, tok_eof};

    failures += countParserExprTestFails(source_tokens);
    return failures;
}

int test_int_expr(){
    fprintf(stderr,"test_int_expr\n");
    std::vector<int> source_tokens = {tok_number_int, tok_eof};
    return countParserExprTestFails(source_tokens);
}

int test_double_expr(){
    fprintf(stderr,"test_double_expr\n");
    std::vector<int> source_tokens = {tok_number_double, tok_eof};
    return countParserExprTestFails(source_tokens);
}

int test_binop_expr(){
    fprintf(stderr,"test_binop_expr\n");
    int failures = 0;
    std::vector<int> source_tokens = {tok_number_int,'+',tok_number_int, tok_eof};

    failures += countParserExprTestFails(source_tokens);

    source_tokens = {tok_number_int,'+',tok_number_int,'+',tok_number_int, tok_eof};

    failures += countParserExprTestFails(source_tokens);
    return failures;
}

int test_paren_expr(){
    fprintf(stderr,"test_paren_expr\n");
    int failures = 0;
    std::vector<int> source_tokens = {'(',tok_number_int,')', tok_eof};

    failures += countParserExprTestFails(source_tokens);

    source_tokens = {'(',tok_number_int,'+',tok_number_int,')', tok_eof};

    failures += countParserExprTestFails(source_tokens);
    return failures;
}

int test_call_expr(){
    fprintf(stderr,"test_call_expr\n");
    int failures = 0;
    std::vector<int> source_tokens = {tok_identifier,'(',')', tok_eof};

    failures += countParserExprTestFails(source_tokens);

    source_tokens = {tok_identifier,'(',tok_number_int,')', tok_eof};

    failures += countParserExprTestFails(source_tokens);

    source_tokens = {tok_identifier,'(',tok_identifier, ',', tok_identifier, ')', tok_eof};

    failures += countParserExprTestFails(source_tokens);
    return failures;
}

int countParserStatementTestFails(std::vector<int> source_tokens){
    int failures = 0;
    utils::setupParseSourceTokens(source_tokens);
    int result = Parser::mainLoop();
    failures += check(result);
    return failures;
}

int test_block_st(){
    fprintf(stderr,"test_block_st\n");
    int failures = 0;
    std::vector<int> source_tokens = {'{','}', tok_eof};

    failures += countParserStatementTestFails(source_tokens);

    source_tokens = {'{',tok_identifier, '=', tok_identifier,';','}', tok_eof};

    failures += countParserStatementTestFails(source_tokens);

    source_tokens = {'{',tok_identifier, '=', tok_identifier,';',tok_identifier,'=', tok_number_double, '+', tok_number_double,';','}', tok_eof};

    failures += countParserStatementTestFails(source_tokens);
    return failures;
}

int test_call_st(){
    fprintf(stderr,"test_call_st\n");
    int failures = 0;
    std::vector<int> source_tokens = {tok_identifier,'(',')',';', tok_eof};

    failures += countParserStatementTestFails(source_tokens);

    source_tokens = {tok_identifier,'(',tok_identifier,')',';', tok_eof};

    failures += countParserStatementTestFails(source_tokens);

    source_tokens = {tok_identifier,'(',tok_identifier,',',tok_number_double,')',';', tok_eof};;

    failures += countParserStatementTestFails(source_tokens);
    return failures;
}

int test_init_st(){
    fprintf(stderr,"test_init_st\n");
    std::vector<int> source_tokens = {tok_identifier,tok_of,tok_bool,'=',tok_true,';', tok_eof};
    int failures = countParserStatementTestFails(source_tokens); 
    return failures;
}

int test_assign_st(){
    fprintf(stderr,"test_assign_st\n");
    std::vector<int> source_tokens = {tok_identifier,'=',tok_true,';', tok_eof};
    int failures = countParserStatementTestFails(source_tokens); 
    return failures;
}

int test_if_st(){
    fprintf(stderr,"test_if_st\n");
    std::vector<int> source_tokens = {
        tok_if,'(',tok_identifier,')',
            '{',tok_identifier, '=', tok_identifier,';','}',
        tok_else,
            '{',tok_identifier, '=', tok_identifier,';','}',
        tok_eof};
    int failures = countParserStatementTestFails(source_tokens); 
    return failures;
}

int test_for_st(){
    fprintf(stderr,"test_for_st\n");
    int failures = 0;
    std::vector<int> source_tokens;

    source_tokens = {
        tok_for,'(',tok_identifier,'=',tok_number_int,';',tok_identifier,'<',tok_number_int,';',tok_identifier,'=',tok_identifier,'+',tok_number_int,';',')',
            '{',tok_identifier, '=', tok_identifier,';','}',
        tok_eof};
    failures += countParserStatementTestFails(source_tokens); 

    source_tokens = {
        tok_for,'(',tok_identifier,'(',')',';',tok_identifier,'<',tok_number_int,';',tok_identifier,'(',')',';',')',
            '{',tok_identifier, '=', tok_identifier,';','}',
        tok_eof};
    failures += countParserStatementTestFails(source_tokens); 

    source_tokens = {
        tok_for,'(',tok_identifier,tok_of,tok_int,'=',tok_number_int,';',tok_identifier,'<',tok_number_int,';',tok_identifier,'(',')',';',')',
            '{',tok_identifier, '=', tok_identifier,';','}',
        tok_eof};
    failures += countParserStatementTestFails(source_tokens);

    return failures;
}


int test_while_st(){
    fprintf(stderr,"test_while_st\n");
    std::vector<int> source_tokens = {
        tok_while,'(',tok_identifier,')',
            '{',tok_identifier, '=', tok_identifier,'+',tok_number_int,';','}',
        tok_eof};
    int failures = countParserStatementTestFails(source_tokens); 
    return failures;
}

int test_return_st(){
    fprintf(stderr,"test_return_st\n");
    std::vector<int> source_tokens = {tok_return, tok_identifier,';', tok_eof};
    int failures = countParserStatementTestFails(source_tokens); 
    return failures;
}

int test_definition(){
    fprintf(stderr,"test_definition\n");
    int failures = 0;
    std::vector<int> source_tokens;

    source_tokens = {
    tok_define, tok_identifier,'(',')',tok_as,
        '{',tok_identifier,'=',tok_identifier,';','}',
    tok_eof};
    failures += countParserStatementTestFails(source_tokens); 

    source_tokens = {
    tok_define, tok_identifier,'(',tok_identifier,tok_of,tok_bool,')',tok_as,
        '{',tok_identifier,'=',tok_identifier,';','}',
    tok_eof};
    failures += countParserStatementTestFails(source_tokens);

    source_tokens = {
    tok_define, tok_identifier,'(',tok_identifier,tok_of,tok_bool,',',tok_identifier,tok_of,tok_int,')',tok_as,
        '{',tok_identifier,'=',tok_identifier,';','}',
    tok_eof};
    failures += countParserStatementTestFails(source_tokens);

    source_tokens = {
    tok_define, tok_identifier,'(',')',tok_gives,tok_bool,tok_as,
        '{',tok_identifier,'=',tok_identifier,';',tok_return, tok_false,';','}',
    tok_eof};
    failures += countParserStatementTestFails(source_tokens); 

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

int test_statements(){
    test_block_st();
    test_call_st();
    test_init_st();
    test_assign_st();
    test_if_st();
    test_for_st();
    test_while_st();
    test_return_st();
    return 0;
}

int test_definitions(){
    test_definition();
    return 0;
}


int test_parser(){
    //Parser::setVerbosity(1);
    utils::setupParserSource();
    test_expressions();
    test_statements();
    test_definitions();
    return 0;
}

} // namespace test
} // namespace bassoon
