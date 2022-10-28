#include "lexer.hxx"
#include "tokens.hxx"

namespace bassoon{

SourceLoc Lexer::lexer_loc_ = {1,0};
std::string Lexer::identifier_ = "";
int Lexer::int_val_ = 0;
double Lexer::double_val_ = 0.0;
bool Lexer::bool_val_ = false;

int Lexer::nextChar(){
    int character = getchar();
    if (character == '\n' || character == '\r'){
        lexer_loc_.line++;
        lexer_loc_.collumn = 0;
    }
    else{
        lexer_loc_.collumn++;
    }
    return character;
}

int Lexer::check_keyword(std::string candidate_token){
    if (identifier_ == "define")
        return tok_define;
    if (identifier_ == "gives")
        return tok_gives;
    if (identifier_ == "as")
        return tok_as;
    if (identifier_ == "return")
        return tok_return;

    if (identifier_ == "of")
        return tok_of;
    if (identifier_ == "int")
        return tok_int;
    if (identifier_ == "double")
        return tok_double;
    if (identifier_ == "bool")
        return tok_bool;
    
    if (identifier_ == "true")
        return tok_true;
    if (identifier_ == "false")
        return tok_false;

    if (identifier_ == "if")
        return tok_if;
    if (identifier_ == "else")
        return tok_else;
    if (identifier_ == "for")
        return tok_for;
    if (identifier_ == "while")
        return tok_while;

    if (identifier_ == "not")
        return tok_not;
    if (identifier_ == "and")
        return tok_and;
    if (identifier_ == "or")
        return tok_or;
    if (identifier_ == "xor")
        return tok_xor;
    if (identifier_ == "nor")
        return tok_nor;
    return tok_identifier;
}

int Lexer::nextTok(){
    static int last_character = ' ';
    while (isspace(last_character))
        last_character = nextChar();

    // Identifiers: [a-zA-Z][a-zA-Z0-9]
    // Also must check for keywords
    if(isalpha(last_character)){
        identifier_ = last_character;
        while(isalnum(last_character=nextChar())){
            identifier_ += last_character;
        }
        return check_keyword(identifier_);
    }

    // Numbers [0-9]*.[0-9]*
    if (isdigit(last_character) || last_character == '.'){
        bool has_decimal = false;
        std::string num_string;
        do {
            has_decimal = has_decimal || last_character == '.';
            num_string += last_character;
            last_character = nextChar();
        } while (isdigit(last_character) || (last_character == '.' && !has_decimal));
        if (has_decimal){
            double_val_ = strtod(num_string.c_str(),nullptr);
            return tok_number_double;
        }
        else{
            // can specify base here so can allow 0x or 0b
            int_val_ = strtol(num_string.c_str(), nullptr, 10);
            return tok_number_int;
        } 
    }

    // Comments [#][_]*
    if (last_character == '#'){
        do
        {
            last_character = nextChar();
        } while (last_character != EOF && last_character != '\n' && last_character != '\n');
        
        if (last_character != EOF)
            return nextTok();
    }

    // End of file
    if (last_character == EOF)
        return tok_eof;

    // Otherwise the character's ascii code
    int this_character = last_character;
    last_character = nextChar();
    return this_character;
}

} // end namespace bassoon