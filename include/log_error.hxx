#ifndef Bassoon_include_log_error_HXX
#define Bassoon_include_log_error_HXX

#include "spdlog/spdlog.h"

namespace bassoon {

template<class T> 
T LogError(std::string str){
    spdlog::error("Error: {0}", str);
    return nullptr;
}

} // namespace bassoon

#endif // Bassoon_include_log_error_HXX