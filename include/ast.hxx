#ifndef Bassoon_include_ast_HXX
#define Bassoon_include_ast_HXX

#include "llvm/IR/BasicBlock.h"
#include "source_loc.hxx"
#include "types.hxx"

namespace bassoon
{

//----------------------
// Base Expression class
//----------------------

class ExprAST{
    SourceLoc loc_;
public:
    ExprAST(SourceLoc loc) : loc_(loc) {};
    virtual ~ExprAST() = default;
    virtual llvm::Value *codegen() = 0;
    int getLine() const {return loc_.line;};
    int getCol() const {return loc_.collumn;};
};

//----------------------
// Value Expressions
//----------------------

class ValueExprAST : public ExprAST {
    BType type_;
public: 
    ValueExprAST(SourceLoc loc, BType type)
        : ExprAST(loc), type_(type) {};
};

class BoolExprAST : public ValueExprAST {
    bool value_;
public:
    BoolExprAST(SourceLoc loc, bool value) 
        : ValueExprAST(loc, type_bool), value_(value) {};
    llvm::Value *codegen() override;
};

class IntExprAST : public ValueExprAST{
    int value_;
public:
    IntExprAST(SourceLoc loc, int value) 
        : ValueExprAST(loc, type_int), value_(value) {};
    llvm::Value *codegen() override;
};

class DoubleExprAST : public ValueExprAST{
    double value_;
public:
    DoubleExprAST(SourceLoc loc, double value) 
        : ValueExprAST(loc, type_double), value_(value) {};
    llvm::Value *codegen() override;
};

//-----------------------
// Variable Expressions
//-----------------------

class VariableExprAST : public ExprAST{
    std::string name_;
    BType type_;
public:
    VariableExprAST(SourceLoc loc, const std::string name, BType type) 
        : ExprAST(loc), name_(name), type_(type) {};
    llvm::Value *codegen() override;
    std::string getName() {return name_;};
    BType getType() {return type_;};
};

//-----------------------
// Operation expressions
//-----------------------

class UnaryExprAST : public ExprAST {
    char opcode_;
    std::unique_ptr<ExprAST> operand_;
public:
    UnaryExprAST(SourceLoc loc, char opcode, std::unique_ptr<ExprAST> operand) 
        : ExprAST(loc), opcode_(opcode), operand_(std::move(operand)) {};
    llvm::Value *codegen() override;
};

class BinaryExprAST : public ExprAST {
    char opcode_;
    std::unique_ptr<ExprAST> lhs_, rhs_;
public:
    BinaryExprAST(SourceLoc loc, char opcode, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs)
        : ExprAST(loc), opcode_(opcode), lhs_(std::move(lhs)), rhs_(std::move(rhs)) {};
};

//--------------------------
// Control Flow Expressions
//--------------------------

class IfExprAST : public ExprAST {
    std::unique_ptr<ExprAST> cond_, then_, else_;
public:
    IfExprAST(SourceLoc loc, std::unique_ptr<ExprAST> cond, std::unique_ptr<ExprAST> then, std::unique_ptr<ExprAST> elsewise) 
        : ExprAST(loc), cond_(std::move(cond)), then_(std::move(then)), else_(std::move(elsewise)) {};
    llvm::Value *codegen() override;
};

class ForExprAST : public ExprAST {
    //std::vector<std::string> ind_var_names;
    std::string ind_var_name_;
    //BType ind_var_type_;
    std::unique_ptr<ExprAST> start_, end_, step_, body_;
public:
    ForExprAST(SourceLoc loc, const std::string &ind_var_name, std::unique_ptr<ExprAST> start, std::unique_ptr<ExprAST> end, std::unique_ptr<ExprAST> step, std::unique_ptr<ExprAST> body)
        : ExprAST(loc), ind_var_name_(ind_var_name), start_(std::move(start)), end_(std::move(end)), step_(std::move(step)), body_(std::move(body)) {};
    llvm::Value *codegen() override;
};

class WhileExprAST : public ExprAST {
    std::unique_ptr<ExprAST> cond_, body_;
public:
    WhileExprAST(SourceLoc loc, std::unique_ptr<ExprAST> cond, std::unique_ptr<ExprAST> body)
        : ExprAST(loc), cond_(std::move(cond)), body_(std::move(body)) {};
    llvm::Value *codegen() override;
};

//-------------------------
// Function Expressions
//-------------------------

class CallExprAST : public ExprAST {
    std::string callee_;
    std::vector<std::unique_ptr<ExprAST>> args_;
public:
    CallExprAST(SourceLoc loc, const std::string &callee, std::vector<std::unique_ptr<ExprAST>> args) 
        : ExprAST(loc), callee_(callee), args_(std::move(args)) {};
    llvm::Value *codegen() override;
};

class PrototypeAST {
    SourceLoc loc_;
    std::string name_;
    std::vector<std::string> args_;
public:
    PrototypeAST(SourceLoc loc, const std::string &name, std::vector<std::string> args)
        : loc_(loc), name_(name), args_(args) {};
    const std::string &getName() const {return name_;};
    llvm::Function *codegen();
};

class FunctionAST{
    std::unique_ptr<PrototypeAST> proto_;
    std::unique_ptr<ExprAST> body_;
public:
    FunctionAST(std::unique_ptr<PrototypeAST> proto, std::unique_ptr<ExprAST> body)
        : proto_(std::move(proto)), body_(std::move(body)) {};
    llvm::Function *codegen();
};



} // namespace bassoon

#endif // Bassoon_include_ast_HXX