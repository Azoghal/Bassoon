#ifndef Bassoon_include_types_HXX
#define Bassoon_include_types_HXX

#include <string>

namespace bassoon{

enum BType {
    type_void = 0,
    type_bool = 1,
    type_int = 2,
    type_double = 3,
};

static std::string typeToStr(int t){
    switch (t)
    {
    case type_void : return "void";
    case type_bool : return "bool";
    case type_int : return "int";
    case type_double : return "double";
    default: return "not a type";
    }
}

} // end namespace bassoon

#endif // Bassoon_include_types_HXX