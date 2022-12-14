#include <exception>

class BError : public std::exception {
public:
    const char * what() const throw() {
        return "Base bassoon exception";
    }
};

// class BParseError : public BError {
// public:
//     char * what () override {
//         return "Parsing error";
//     }
// };
