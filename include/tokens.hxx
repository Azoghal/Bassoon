#ifndef Bassoon_include_tokens_HXX
#define Bassoon_include_tokens_HXX

#include "types.hxx"

namespace bassoon{

enum token {
    tok_eof = -1,
    tok_eol = -2,
    
    //functions
    tok_define = -3,
    tok_gives = -4,
    tok_as = -5,
    tok_return = -6,

    //types
    tok_of = -7,
    tok_int = -8,
    tok_double = -9,
    tok_bool = -10,

    //primary
    tok_identifier = -11,
    tok_number_int = -12,
    tok_number_double = -13,
    tok_true = -14,
    tok_false = -15,

    //control
    tok_if = -16,
    tok_else = -17,
    tok_for = -18,
    tok_while = -19,
    
    //word operators
    tok_not = -20,
    tok_and = -21,
    tok_or = -22,
    tok_xor = -23,
    tok_nor = -24,
};

static std::string tokToStr(int t){
    switch (t)
    {
    case tok_eof : return "tok_eof";
    case tok_eol : return "tok_eol";
    case tok_define : return "tok_define";
    case tok_gives : return "tok_gives";
    case tok_as : return "tok_as";
    case tok_return : return "tok_return";

    //types
    case tok_of : return "tok_of";
    case tok_int : return "tok_int";
    case tok_double : return "tok_double";
    case tok_bool : return "tok_bool";

    //primary
    case tok_identifier : return "tok_identifier";
    case tok_number_int : return "tok_number_int";
    case tok_number_double : return "tok_number_double";
    case tok_true : return "tok_true";
    case tok_false : return "tok_false";

    //control
    case tok_if : return "tok_if";
    case tok_else : return "tok_else";
    case tok_for : return "tok_for";
    case tok_while : return "tok_while";
    
    //word operators
    case tok_not : return "tok_not";
    case tok_and : return "tok_and";
    case tok_or : return "tok_or";
    case tok_xor : return "tok_xor";
    case tok_nor : return "tok_nor";
    default: return "not a token";
    }
}

static int isType(int tok){
    if (tok == tok_bool ||
        tok == tok_int ||
        tok == tok_double)
        return 1;
    else
        return 0;
}

static int tokToType(int tok){
    switch(tok)
    {
    default: return 0;
    case tok_bool: return type_bool;
    case tok_int: return type_int;
    case tok_double: return type_double;
    }

}

} // end namespace bassoon

#endif // Bassoon_include_tokens_HXX