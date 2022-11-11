#ifndef Bassoon_src_test_test_utils_HXX
#define Bassoon_src_test_test_utils_HXX

#include <string>

namespace bassoon{
namespace test{


std::string input_string;
int input_index, input_end;

void setup_input_string(std::string input){
    input_string = input;
    input_index = 0;
    input_end = input_string.size();
}

int mock(){
    return input_string[input_index++];
}

} // namespace test
} // namespace bassoon

#endif // Bassoon_src_test_test_utils_HXX
