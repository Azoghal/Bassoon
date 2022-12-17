#ifndef Bassoon_include_types_HXX
#define Bassoon_include_types_HXX

#include <string>

namespace bassoon{

enum BType {
    type_void = -2,
    not_a_type = -1,
    type_unknown = 0,
    type_bool = 1,
    type_int = 2,
    type_double = 3,
};

class BFType{
    std::vector<BType> argument_types_; // can be const/final? 
    BType return_type_;
    bool valid_;
public:
    BFType(): valid_(false) {}
    BFType(std::vector<BType> argument_types, BType return_type) 
        : argument_types_(argument_types), return_type_(return_type), valid_(true) {}
    const std::vector<BType> & getArgumentTypes(){return argument_types_;}
    const BType & getReturnType() const {return return_type_;}
    int getArgCount(){return argument_types_.size();}
    bool isValid(){return valid_;}
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