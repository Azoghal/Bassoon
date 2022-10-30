#include "lexer.hxx"
#include "tokens.hxx"
#include <iostream>

/*
def foo(a of int) gives bool as {if a>10 {return true} else {return false}
*/

int main(){   
    while(true){
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
