#ifndef Bassoon_include_log_error_HXX
#define Bassoon_include_log_error_HXX

template<class T> 
T LogError(const char *str){
    fprintf(stderr, "Error %s\n", str);
    return nullptr;
}

#endif // Bassoon_include_log_error_HXX