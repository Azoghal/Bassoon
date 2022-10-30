#include <iostream>
#include <functional>
#include <string.h>

#include "str_utils.hxx"
#include "../lexer.hxx"
#include "../tokens.hxx"
#include "lexer_test.hxx"

std::string input_string;
int input_index, input_end;

void setup_input_string(std::string input){
    input_string = input;
    input_index = 0;
    input_end = input_string.size();
}

int mock(){
    return input_string[input_index++];
}

static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
            std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
}

std::vector<int> GetLexedTokens(std::string input){
    //trim and add a newline to stop interference with next test
    str_utils::rtrim(input);
    input+="\n";
    setup_input_string(input);
    std::vector<int> result;
    while(input_index < input_end){
        int t = bassoon::Lexer::nextTok();
        result.push_back(t);
    }
    // for(int i : result){
    //     fprintf(stderr, "%d: %s\n", i, bassoon::tok_to_str(i).c_str());
    // }
    return result;
}

int verifyExpectedTokens(std::vector<int> to_check, std::vector<int> expected){
    int c_size = to_check.size();
    int e_size = expected.size();
    
    for (int i=0; i<c_size && i<e_size; ++i){
        if (to_check[i] != expected[i]){
            fprintf(stderr, "Element %d of lexed tokens (%d: %s) differs from expected (%s).\n", i,to_check[i],bassoon::tok_to_str(to_check[i]).c_str(), bassoon::tok_to_str(expected[i]).c_str());
            return 1;
        }
    }
    if (c_size != e_size){
        fprintf(stderr, "Lexed tokens of size %d differs from expected tokens of size %d\n", c_size, e_size);
        fprintf(stderr,"Lexed\n");
        for(int i : to_check)
            fprintf(stderr,"%s, ", bassoon::tok_to_str(i).c_str());
        fprintf(stderr,"\nExpected\n");
        for(int i : expected)
            fprintf(stderr,"%s, ", bassoon::tok_to_str(i).c_str());
        return 1;
    }
    fprintf(stderr, "pass\n");
    return 0;
}

int countFailedCases(std::vector<std::string> test_cases, std::vector<int> expected_tokens){
    int tests_failed = 0;
    for (std::string test_case : test_cases){
        std::vector<int> lexed_tokens = GetLexedTokens(test_case);
        tests_failed += verifyExpectedTokens(lexed_tokens, expected_tokens);
    }
    return tests_failed;
}

int countFailedCases(std::vector<std::string> test_cases, std::vector<std::vector<int>> expected_tokens_list){
    // Overload of countFailedCases that allows for different results
    if (test_cases.size() != expected_tokens_list.size()){
        fprintf(stderr, "unequal size of example and expected vectors");
        return 1;
    }
    int tests_failed = 0;
    for (int i=0; i<test_cases.size(); ++i){
        std::vector<int> lexed_tokens = GetLexedTokens(test_cases[i]);
        tests_failed += verifyExpectedTokens(lexed_tokens, expected_tokens_list[i]);
    }
    return tests_failed;
}

namespace bassoon{

int test::test_immediate_double(){
    fprintf(stderr, "test_immediate_double\n");
    std::vector<std::string> double_examples = {
        "0.1",
        "2.",
        ".3"
    };
    std::vector<int> expected_tokens = {tok_number_double};
    return countFailedCases(double_examples, expected_tokens);
}

int test::test_immediate_int(){
    fprintf(stderr, "test_immediate_int\n");
    std::vector<std::string> int_examples = {
        "0",
        "1240500"
    };
    std::vector<int> expected_tokens = {tok_number_int};
    return countFailedCases(int_examples, expected_tokens);
}

int test::test_immediate_bool(){
    fprintf(stderr, "test_immediate_bool\n");
    std::vector<std::string> int_examples = {
        "false",
        "true"
    };
    std::vector<std::vector<int>> expected_tokens_list = {{tok_false},{tok_true}};
    return countFailedCases(int_examples, expected_tokens_list);
}

int test::test_typed_variables(){
    fprintf(stderr, "test_typed_variables\n");
    std::vector<std::string> typed = {
        "a of int",
        "longerName of double",
        "AnotherLongName of bool"
    };
    std::vector<std::vector<int>> expected_tokens_list = {
        {tok_identifier, tok_of, tok_int},
        {tok_identifier, tok_of, tok_double},
        {tok_identifier, tok_of, tok_bool}
    };
    return countFailedCases(typed, expected_tokens_list);
}

int test::test_function_def(){
    fprintf(stderr, "test_function_def\n");
    std::vector<std::string> equivalent_definitions = {
        "def foo ( a of int ) gives double as { return 0.0 }",
        "def foo(a of int)gives double as{return 0.0}"
    };
    std::vector<int> expected_tokens = {tok_define, tok_identifier,'(', tok_identifier, tok_of, tok_int, ')', tok_gives, tok_double, tok_as, '{',tok_return, tok_number_double,'}'};
    return countFailedCases(equivalent_definitions, expected_tokens);
}

int test::test_for_loop(){
    fprintf(stderr, "test_for_loop\n");
    std::vector<std::string> equivalent_loops = {
        "for ( a of int = 0; a < 10; a = a +1) { } ",
        "for(a of int=0;a<10;a=a+1){}"
    };
    std::vector<int> expected_tokens = {tok_for, '(', tok_identifier, tok_of, tok_int, '=',tok_number_int,';',tok_identifier,'<',tok_number_int,';',tok_identifier,'=',tok_identifier,'+',tok_number_int,')','{','}'};
    return countFailedCases(equivalent_loops, expected_tokens);
}

int test::test_while_loop(){
    fprintf(stderr, "test_while_loop\n");
    std::vector<std::string> equivalent_loops = {
        "while ( a < 10) {b = b + a;} ",
        "while(a<10){b=b+a;}"
    };
    std::vector<int> expected_tokens = {tok_while, '(', tok_identifier, '<',tok_number_int, ')','{',tok_identifier, '=', tok_identifier,'+',tok_identifier,';','}'};
    return countFailedCases(equivalent_loops, expected_tokens);
}
    
int test::test_lexer(){
    Lexer::setSource(mock);
    test_immediate_int();
    test_immediate_double();
    test_immediate_bool();
    test_typed_variables();
    test_function_def();
    test_for_loop();
    test_while_loop();
    return 0;
}

} // namespace bassoon
