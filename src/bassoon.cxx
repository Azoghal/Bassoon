#include "lexer.hxx"
#include "parser.hxx"
#include "tokens.hxx"
#include <iostream>

/*
define foo(a of int) gives bool as {if a>10 {return true} else {return false}
*/

/*
define boolFoo(b of bool) gives bool as {return true;}
define boolBaz() as {a=false;}
*/

int main(){
    int a = 1;
    // while(a >= 0)
    //     a = bassoon::Lexer::nextTok(); // skip newline
    // fprintf(stderr, "first token is a %c %s\n", a, bassoon::tokToStr(a).c_str());
    bassoon::Parser::mainLoop();
    a++;
    return 0;
}
