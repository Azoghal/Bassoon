#ifndef Bassoon_lexer_HXX
#define Bassoon_lexer_HXX

#include "source_loc.hxx"
#include <string>

namespace bassoon {
class BassoonLexer{
    static SourceLoc lexer_loc_;
    static std::string identifier_;
    static double double_val_;
    static int int_val_;
    static bool bool_val_;
    static int check_keyword(std::string candidate_token);
public:
    BassoonLexer();
    static int nextChar();
    static int nextTok();
};


} // end namespace bassoon


#endif // Bassoon_lexer_HXX