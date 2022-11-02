#ifndef Bassoon_include_lexer_HXX
#define Bassoon_include_lexer_HXX

#include "source_loc.hxx"
#include <string>
#include <functional>

namespace bassoon {
class Lexer{
    static std::function<int()> bassoon_getchar_;
    static SourceLoc lexer_loc_;
    static std::string identifier_;
    static double double_val_;
    static int int_val_;
    static bool bool_val_; // redundant?
    static int check_keyword(std::string candidate_token);
public:
    Lexer() {};
    static void setSource(std::function<int()> source);
    static int nextChar();
    static int nextTok();
    static std::string getIdentifier();
    static double getDouble();
    static int getInt();
    static SourceLoc getLoc();
};


} // end namespace bassoon


#endif // Bassoon_include_lexer_HXX