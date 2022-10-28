#include <iostream>
#include <functional>
#include <string.h>

#include "../lexer.hxx"
#include "../tokens.hxx"
#include "lexer_test.hxx"


std::string input_chars = "def foo(a of int) gives double as {return 0.0};";
int a = 0;
int end = input_chars.size();

int mock(){
    return input_chars[a++];
}

int main(){
    bassoon::Lexer::setSource(mock);
    while(a < end){
        int a = bassoon::Lexer::nextTok();
        if (a<0){
            fprintf(stderr, "%s\n", bassoon::tok_to_str(a).c_str());
        }
        else{
            char c = a;
            fprintf(stderr, "%c\n", c);
        }
        if (a == bassoon::tok_eof)
            break;
    }
    return 0;
}