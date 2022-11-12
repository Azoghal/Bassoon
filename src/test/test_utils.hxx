#ifndef Bassoon_src_test_test_utils_HXX
#define Bassoon_src_test_test_utils_HXX

#include <string>

#include "lexer.hxx"
#include "parser.hxx"
#include "tokens.hxx"

namespace bassoon{
namespace test{
namespace utils{

// ------ Lexer Test Mocking ------

static std::string lexer_source_string;
static int lexer_source_index, lexer_source_end;

static void setupLexerSourceString(std::string input){
    lexer_source_string = input;
    lexer_source_index = 0;
    lexer_source_end = lexer_source_string.size();
}

static int lexerMock(){
    return lexer_source_string[lexer_source_index++];
}

static void setupLexerSource(){
    Lexer::setSource(lexerMock);
}

// ------ Parser Test Mocking ---------

static std::vector<int> parserTokens;
static int parserTokensIndex, parserTokensEnd;

static void setupParseSourceTokens(std::vector<int> tokens){
    
    parserTokens = tokens;
    parserTokensIndex = 0;
    parserTokensEnd = parserTokens.size()-1;
    // reset parser to be fresh each time.
    Parser::_testResetCurrentToken();
    if(parserTokens[parserTokens.size()-1] != tok_eof){
        fprintf(stderr,"Warning, no tok_eof to end token vector");
    } 
}

static int parserMock(){
    if (parserTokensIndex >= parserTokens.size()){
        fprintf(stderr, "parserMock gone beyond given tokens. Ensure tok_eof terminates tokens vector.\n");
        return tok_eof;
    }
    return parserTokens[parserTokensIndex++];
}

static void setupParserSource(){
    Parser::setSource(parserMock);
}

} // namespace utils
} // namespace test
} // namespace bassoon

#endif // Bassoon_src_test_test_utils_HXX
