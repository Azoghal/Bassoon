#include <iostream>
#include <functional>
#include <string.h>

#include "../lexer.hxx"
#include "../tokens.hxx"
#include "lexer_test.hxx"

std::string input_string;
int input_index, input_end;

void setup_input_string(std::string input){
    input_string = "def foo(a of int) gives double as {return 0.0};";
    input_index = 0;
    input_end = input_string.size();
}

int mock(){
    return input_string[input_index++];
}

std::vector<int> GetLexedTokens(std::string input){
    setup_input_string(input);
    std::vector<int> result;
    while(input_index < input_end){
        int t = bassoon::Lexer::nextTok();
        //fprintf(stderr, "%s\n", bassoon::tok_to_str(t).c_str());
        result.push_back(t);
    }
    return result;
}

int verifyExpectedTokens(std::vector<int> to_check, std::vector<int> expected){
    int c_size = to_check.size();
    int e_size = expected.size();
    for (int i=0; i<c_size && i<e_size; ++i){
        if (to_check[i] != expected[i]){
            fprintf(stderr, "Element %d of lexed tokens (%s) differs from expected (%s).", i, bassoon::tok_to_str(to_check[i]).c_str(), bassoon::tok_to_str(expected[i]).c_str());
            return 1;
        }
    }
    if (c_size != e_size){
        fprintf(stderr, "Lexed tokens of size %d differs from expected tokens of size %d", c_size, e_size);
        return 1;
    }
    fprintf(stderr, "pass\n");
    return 0;
}

namespace bassoon{

int test::test_function_def(){
    std::string function_def = "def foo ( a of int ) gives double as { return 0.0 }";
    std::vector<int> expected_tokens = {tok_def, tok_identifier,'(', tok_identifier, tok_of, tok_int, ')', tok_gives, tok_double, tok_as, '{',tok_return, tok_number_double,'}'};
    std::vector<int> lexed_tokens = GetLexedTokens(function_def);
    return verifyExpectedTokens(lexed_tokens, expected_tokens);
}
    
int test::test_lexer(){
    Lexer::setSource(mock);
    test_function_def();
    return 0;
}

} // namespace bassoon
