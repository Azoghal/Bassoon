#ifndef Bassoon_include_ast_HXX
#define Bassoon_include_ast_HXX

#include "llvm/IR/BasicBlock.h"
#include "source_loc.hxx"
#include "types.hxx"

namespace bassoon
{

// class NodeAST{
//     SourceLoc loc_;
// public:
//     NodeAST(SourceLoc loc) : loc_(loc) {};
//     virtual ~NodeAST() = default;
//     virtual llvm::Value *codegen() = 0;
//     int getLine() const {return loc_.line;};
//     int getCol() const {return loc_.collumn;};
// };

//----------------------
// Base Expression class
//----------------------

class ExprAST {
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
// Identifier Expressions
//-----------------------

// reference
class VariableExprAST : public ExprAST{
    std::string name_;
public:
    VariableExprAST(SourceLoc loc, const std::string name) 
        : ExprAST(loc), name_(name) {};
    llvm::Value *codegen() override;
    std::string getName() {return name_;};
};

class CallExprAST : public ExprAST {
    std::string callee_;
    std::vector<std::unique_ptr<ExprAST>> args_;
public:
    CallExprAST(SourceLoc loc, const std::string &callee, std::vector<std::unique_ptr<ExprAST>> args) 
        : ExprAST(loc), callee_(callee), args_(std::move(args)) {};
    llvm::Value *codegen() override;
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
    llvm::Value *codegen() override;
};
// -------------------------
// Statements
// -------------------------

class StatementAST{
    SourceLoc loc_;
public:
    StatementAST(SourceLoc loc) : loc_(loc) {};
    virtual ~StatementAST() = default;
    virtual llvm::Value *codegen() = 0;
    int getLine() const {return loc_.line;};
    int getCol() const {return loc_.collumn;};
};

//--------------------------
// Control Flow Statements
//--------------------------

class IfStatementAST : public StatementAST {
    std::unique_ptr<ExprAST> cond_;
    std::unique_ptr<StatementAST> then_, else_;
public:
    IfStatementAST(SourceLoc loc, std::unique_ptr<ExprAST> cond, std::unique_ptr<StatementAST> then, std::unique_ptr<StatementAST> elsewise) 
        : StatementAST(loc), cond_(std::move(cond)), then_(std::move(then)), else_(std::move(elsewise)) {};
    llvm::Value *codegen() override;
};

class ForStatementAST : public StatementAST {
    //std::vector<std::string> ind_var_names;
    std::string ind_var_name_;
    //BType ind_var_type_;
    std::unique_ptr<ExprAST> end_;
    std::unique_ptr<StatementAST> start_, step_, body_;
public:
    ForStatementAST(SourceLoc loc, const std::string &ind_var_name, std::unique_ptr<StatementAST> start, std::unique_ptr<ExprAST> end, std::unique_ptr<StatementAST> step, std::unique_ptr<StatementAST> body)
        : StatementAST(loc), ind_var_name_(ind_var_name), start_(std::move(start)), end_(std::move(end)), step_(std::move(step)), body_(std::move(body)) {};
    llvm::Value *codegen() override;
};

class WhileStatementAST : public StatementAST {
    std::unique_ptr<ExprAST> cond_;
    std::unique_ptr<StatementAST> body_;
public:
    WhileStatementAST(SourceLoc loc, std::unique_ptr<ExprAST> cond, std::unique_ptr<StatementAST> body)
        : StatementAST(loc), cond_(std::move(cond)), body_(std::move(body)) {};
    llvm::Value *codegen() override;
};

class ReturnStatementAST : public StatementAST {
    std::unique_ptr<ExprAST> return_expr_;
public:
    ReturnStatementAST(SourceLoc loc, std::unique_ptr<ExprAST> return_expr)
        : StatementAST(loc), return_expr_(std::move(return_expr)) {};
    llvm::Value *codegen() override;
};

// ------------------------
// Other Statements
// -----------------------

class BlockStatementAST : public StatementAST{
    std::vector<std::unique_ptr<StatementAST>> statements_;
public:
    BlockStatementAST(SourceLoc loc, std::vector<std::unique_ptr<StatementAST>> statements)
        : StatementAST(loc), statements_(std::move(statements)) {};
    llvm::Value *codegen() override;
};

class CallStatementAST : public StatementAST {
    std::unique_ptr<CallExprAST> call_;
public:
    CallStatementAST(SourceLoc loc, std::unique_ptr<CallExprAST> call)
        : StatementAST(loc), call_(std::move(call)) {};
    llvm::Value *codegen() override;
};

class AssignStatementAST : public StatementAST {
    std::string identifier_;
    std::unique_ptr<ExprAST> value_;
public:
    AssignStatementAST(SourceLoc loc, std::string identifier, std::unique_ptr<ExprAST> value)
        : StatementAST(loc), identifier_(identifier), value_(std::move(value)) {};
    llvm::Value *codegen() override;
};

class InitStatementAST : public StatementAST{
    std::string identifier_;
    BType var_type_;
    std::unique_ptr<AssignStatementAST> assignment_;
public:
    InitStatementAST(SourceLoc loc, std::string identifier, BType var_type, std::unique_ptr<AssignStatementAST> assignment)
        : StatementAST(loc), identifier_(identifier), var_type_(var_type), assignment_(std::move(assignment)) {};
    llvm::Value *codegen() override;
};

//-------------------------
// Function Expressions
//-------------------------

class PrototypeAST {
    SourceLoc loc_;
    std::string name_;
    std::vector<std::pair<std::string,int>> args_;
    int return_type_;
public:
    PrototypeAST(SourceLoc loc, const std::string &name, std::vector<std::pair<std::string,int>> args, int return_type)
        : loc_(loc), name_(name), args_(args), return_type_(return_type) {};
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