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
    tok_number_int = -11,
    tok_number_double = -12,
    tok_true = -13,
    tok_false = -14,

    //control
    tok_if = -15,
    tok_else = -16,
    tok_for = -17,
    tok_while = -18,
    
    //word operators
    tok_not = -19,
    tok_and = -20,
    tok_or = -21,
    tok_xor = -22,
    tok_nor = -23,
};