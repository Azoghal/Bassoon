#ifndef Bassoon_include_inbuilt_operators_HXX
#define Bassoon_include_inbuilt_operators_HXX

#include "types.hxx"

namespace bassoon
{

std::map<char, std::vector<BFType>> unary_operators = 
{   {'-', 
        {
            {BFType(std::vector<BType>({type_int}), type_int)},
            {BFType(std::vector<BType>({type_double}),type_double)}
        }
    },
    {'!',
        {
            {BFType((std::vector<BType>{type_bool}),type_bool)}
        }
    }
};



std::map<char, std::vector<BFType>> binary_operators = 
{   {'-', 
        {
            {BFType(std::vector<BType>({type_int,type_int}), type_int)},
            {BFType(std::vector<BType>({type_double,type_double}),type_double)},
            {BFType(std::vector<BType>({type_int,type_double}),type_double)},
            {BFType(std::vector<BType>({type_double,type_int}),type_double)},
        }
    },
    {'+', 
        {
            {BFType(std::vector<BType>({type_int,type_int}), type_int)},
            {BFType(std::vector<BType>({type_double,type_double}),type_double)},
            {BFType(std::vector<BType>({type_int,type_double}),type_double)},
            {BFType(std::vector<BType>({type_double,type_int}),type_double)},
        }
    },
    {'*', 
        {
            {BFType(std::vector<BType>({type_int,type_int}), type_int)},
            {BFType(std::vector<BType>({type_double,type_double}),type_double)},
            {BFType(std::vector<BType>({type_int,type_double}),type_double)},
            {BFType(std::vector<BType>({type_double,type_int}),type_double)},
        }
    },
    {'/', 
        {
            {BFType(std::vector<BType>({type_int,type_int}), type_int)},
            {BFType(std::vector<BType>({type_double,type_double}),type_double)},
            {BFType(std::vector<BType>({type_int,type_double}),type_double)},
            {BFType(std::vector<BType>({type_double,type_int}),type_double)},
        }
    },
    {'<', 
        {
            {BFType(std::vector<BType>({type_int,type_int}), type_bool)},
            {BFType(std::vector<BType>({type_double,type_double}),type_bool)},
            {BFType(std::vector<BType>({type_int,type_double}),type_bool)},
            {BFType(std::vector<BType>({type_double,type_int}),type_bool)},
        }
    },
};

} // namespace bassoon

#endif // Bassoon_include_inbuilt_operators_HXX