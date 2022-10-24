# Bassoon
Practice language frontend using LLVM.

## Aims
- `bool`, `int`, `double` types
- static compile time type checking
- `if`, `for`, `while` control flow constructs
- mutable variables
- function definitions
- recursive functions

## Further Aims
- Exceptions
- Arrays

Target Language Syntax
```
def fancyFunc(a of int, b of double) gives int as {
    bool condition
    condition = a == 0 or b > 0.5
    if condition {
        return fancyFunc(a);
    }
    else{
        while not condition {
            b -= 0.1; a -= 1;
            condition = a == 0 or b > 0.5;
        }
        for (int i = 1; i < 12; i++){
            a -= i*b;
        }
        return a;
    }
    return -1;
}
```