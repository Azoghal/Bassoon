#ifndef Bassoon_include_exceptions_HXX
#define Bassoon_include_exceptions_HXX

#include <exception>
#include <string>
#include <fmt/core.h>

// make virtual/abstract?
class BError : public std::exception {
    const std::string base_message_;
public:
    BError(): base_message_("Base bassoon exception"){};
    BError(std::string base_message) : base_message_(base_message){};
    const char * what() const throw() {
        return base_message_.c_str();
    }
};

// also virtual / abstract
class InternalBError : public BError{
public:
    InternalBError(): BError("Internal Base Bassoon Error"){};
    InternalBError(std::string msg): BError(msg){};
};

class TypeContextError : public InternalBError{
public:
    TypeContextError(): InternalBError("Type Context Error"){};
};

class FuncContextError : public InternalBError{
public:
    FuncContextError(): InternalBError("Func Context Error"){};
};

class TypingError : public BError {
public:
    TypingError();
    TypingError(std::string message) : BError(message) {};
};

class InvalidReferenceError : public BError {
    std::string variable_name_;
    std::string loc_str_;
public:
    InvalidReferenceError(std::string var_name, std::string loc_str):
        BError(fmt::format("Var {} referenced at {} before definition", var_name, loc_str)),
        variable_name_(var_name),
        loc_str_(loc_str){};
};

#endif // Bassoon_include_exceptions_HXX