#include "parser.hxx"

namespace bassoon
{

template<class T> 
T Parser::LogError(const char *str){
    fprintf(stderr, "Error %s\n", str);
    return nullptr;
}

} // namespace bassoon
