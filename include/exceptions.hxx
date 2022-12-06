#include <exception>

class BError : public std::exception {
public:
    char * what () {
        return "Base bassoon exception";
    }
};

// class BParseError : public BError {
// public:
//     char * what () override {
//         return "Parsing error";
//     }
// };
