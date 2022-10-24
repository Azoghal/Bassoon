enum token {
    tok_eof = -1,
    tok_eol = -2,
    
    //functions
    tok_def = -3,
    tok_gives = -4,
    tok_as = -5,
    tok_return = -6,

    //types
    tok_int = -7,
    tok_double = -8,
    tok_bool = -9,

    //primary
    tok_identifier = -10,
    tok_number = -11,

    //control
    tok_if = -12,
    tok_else = -13,
    tok_for = -14,
    tok_while = -15,
    
    //word operators
    tok_not = -16,
    tok_and = -17,
    tok_or = -18,
    tok_xor = -19,
    tok_nor = -20,
};