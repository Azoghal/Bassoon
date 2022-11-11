#include <iostream>

#include "lexer_test.hxx"
#include "parse_test.hxx"

int main(){
    // mock getchar??
    // or change lexer to take entire file 
    fprintf(stderr, "====== Testing Lexer ======\n");
    bassoon::test::test_lexer();
    fprintf(stderr,"====== Testing Parser ======\n");
    bassoon::test::test_parser();
    return 0;
}