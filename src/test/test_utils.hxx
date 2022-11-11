#ifndef Bassoon_src_test_test_utils_HXX
#define Bassoon_src_test_test_utils_HXX

#include <string>

#include "lexer.hxx"

namespace bassoon{
namespace test{
namespace utils{

static std::string input_string;
static int input_index, input_end;

static void setup_input_string(std::string input){
    input_string = input;
    input_index = 0;
    input_end = input_string.size();
}

static int mock(){
    return input_string[input_index++];
}

static void setupSource(){
    Lexer::setSource(mock);
}

} // namespace utils
} // namespace test
} // namespace bassoon

#endif // Bassoon_src_test_test_utils_HXX
