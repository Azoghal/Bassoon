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
    int a = bassoon::Lexer::nextTok(); // skip newline
    bassoon::Parser::parseDefinition();
    a++;
    return 0;
}
