# Bassoon
Small procedural language frontend generating LLVM IR.

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

Original Target Language Syntax
```
define fancyFunc(a of int, b of double) gives int as {
    bool condition
    condition = a == 0 or b > 0.5
    if condition {
        return fancyFunc(a)
    }
    else{
        # this is a comment
        while not condition {
            b -= 0.1; a -= 1
            condition = a == 0 or b > 0.5
        }
        for (int i = 1; i < 12; a=a+1){
            a -= i*b # implicit cast
        }
        return a
    }
    return -1
}
```

Current Language Syntax
```
define fancyFunc(a of int, b of double) gives int as {
    bool condition;
    condition = b > 0.5;
    if (condition) {
        return fancyFunc(a);
    }
    else{
        # this is a comment
        while (not condition){
            b = b - 0.1; 
            a = a - 1;
            condition = b > 0.5;
        }
        for (int i = 1; i < 12; a=a+1;){
            a = a - i*b; # implicit cast
        }
        return a;
    }
    return -1;
}
```

Most of these changes are to avoid needless complexity in uninteresting areas (the parser), particularly the use of semicolon terminators and parentheses in control flow structures. A standout feature that is missing is binary boolean operations.

```bash
cd build/bin
./Bassoon < <file path>
clang output.o -o <executable name>
```
