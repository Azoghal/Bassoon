#ifndef Bassoon_src_types_HXX
#define Bassoon_src_types_HXX

#include <string>

namespace bassoon{

enum BType {
    type_bool = 0,
    type_int = 1,
    type_double = 2,
};

static std::string type_to_str(int t){
    switch (t)
    {
    case type_bool : return "bool";
    case type_int : return "int";
    case type_double : return "double";
    default: return "not a type";
    }
}

} // end namespace bassoon

#endif // Bassoon_src_types_HXX