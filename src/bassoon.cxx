#include "lexer.hxx"
#include "tokens.hxx"
#include <iostream>

int main(){
    //bassoon::BassoonLexer lexer;
    int a = bassoon::Lexer::nextChar();
    token val = static_cast<token>(a);
    std::cout << val << "\n";
    return 0;
}