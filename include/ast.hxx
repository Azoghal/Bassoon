#ifndef Bassoon_include_ast_HXX
#define Bassoon_include_ast_HXX

#include "llvm/IR/BasicBlock.h"
#include "source_loc.hxx"
#include "types.hxx"

namespace bassoon
{

class BoolExprAST;
class IntExprAST;
class DoubleExprAST;
class VariableExprAST;
class CallExprAST;
class UnaryExprAST;
class BinaryExprAST;
class IfStatementAST;
class ForStatementAST;
class WhileStatementAST;
class ReturnStatementAST;
class BlockStatementAST;
class CallStatementAST;
class AssignStatementAST;
class InitStatementAST;
class PrototypeAST;
class FunctionAST;

class ASTVisitor{

public:
    virtual void boolExprAction(BoolExprAST * bool_node) = 0;
    virtual void intExprAction(IntExprAST * int_node) = 0;
    virtual void doubleExprAction(DoubleExprAST * double_node) = 0;
    virtual void variableExprAction(VariableExprAST * variable_node) = 0;
    virtual void callExprAction(CallExprAST * call_node) = 0;
    virtual void unaryExprAction(UnaryExprAST * unary_node) = 0;
    virtual void binaryExprAction(BinaryExprAST * binary_node) = 0;
    
    virtual void ifStAction(IfStatementAST * if_node) = 0;
    virtual void forStAction(ForStatementAST * for_node) = 0;
    virtual void whileStAction(WhileStatementAST * while_node) = 0;
    virtual void returnStAction(ReturnStatementAST * return_node) = 0;
    virtual void blockStAction(BlockStatementAST * block_node) = 0;
    virtual void callStAction(CallStatementAST * call_node) = 0;
    virtual void assignStAction(AssignStatementAST * assign_node) = 0;
    virtual void initStAction(InitStatementAST * init_node) = 0;

    virtual void prototypeAction(PrototypeAST * proto_node) = 0;
    virtual void functionAction(FunctionAST * func_node) = 0;
};

class NodeAST{
    SourceLoc loc_;
public:
    NodeAST(SourceLoc loc) : loc_(loc) {};
    virtual ~NodeAST() = default;
    virtual void accept(ASTVisitor * v) = 0;
    int getLine() const {return loc_.line;};
    int getCol() const {return loc_.collumn;};
};

//----------------------
// Base Expression class
//----------------------

class ExprAST : public NodeAST{
public:
    ExprAST(SourceLoc loc) : NodeAST(loc) {};
    virtual ~ExprAST() = default;
    virtual llvm::Value *codegen() = 0;
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
    void accept(ASTVisitor * v) override {v->boolExprAction(this);}
    bool getValue(){return value_;}
};

class IntExprAST : public ValueExprAST{
    int value_;
public:
    IntExprAST(SourceLoc loc, int value) 
        : ValueExprAST(loc, type_int), value_(value) {};
    llvm::Value *codegen() override;
    void accept(ASTVisitor * v) override {v->intExprAction(this);}
    bool getValue(){return value_;}
};

class DoubleExprAST : public ValueExprAST{
    double value_;
public:
    DoubleExprAST(SourceLoc loc, double value) 
        : ValueExprAST(loc, type_double), value_(value) {};
    llvm::Value *codegen() override;
    void accept(ASTVisitor * v) override {v->doubleExprAction(this);}
    bool getValue(){return value_;}
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
    void accept(ASTVisitor * v) override {v->variableExprAction(this);}
    std::string getName() {return name_;};
};

class CallExprAST : public ExprAST {
    std::string callee_;
    std::vector<std::unique_ptr<ExprAST>> args_;
public:
    CallExprAST(SourceLoc loc, const std::string &callee, std::vector<std::unique_ptr<ExprAST>> args) 
        : ExprAST(loc), callee_(callee), args_(std::move(args)) {};
    llvm::Value *codegen() override;
    void accept(ASTVisitor * v) override {v->callExprAction(this);};
    std::string getName(){return callee_;}
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
    void accept(ASTVisitor * v) override {v->unaryExprAction(this);};
    char getOpCode(){return  opcode_;}
    std::shared_ptr<ExprAST> getOperand(){return std::move(operand_);}
};

class BinaryExprAST : public ExprAST {
    char opcode_;
    std::unique_ptr<ExprAST> lhs_, rhs_;
public:
    BinaryExprAST(SourceLoc loc, char opcode, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs)
        : ExprAST(loc), opcode_(opcode), lhs_(std::move(lhs)), rhs_(std::move(rhs)) {};
    llvm::Value *codegen() override;
    void accept(ASTVisitor * v) override {v->binaryExprAction(this);};
    char getOpCode(){return opcode_;};
    std::shared_ptr<ExprAST> getLHS(){return std::move(lhs_);}
    std::shared_ptr<ExprAST> getRHS(){return std::move(rhs_);}
};
// -------------------------
// Statements
// -------------------------

class StatementAST : public NodeAST{
public:
    StatementAST(SourceLoc loc) : NodeAST(loc) {};
    virtual ~StatementAST() = default;
    virtual llvm::Value *codegen() = 0;
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
    void accept(ASTVisitor * v) override {v->ifStAction(this);};
};

class ForStatementAST : public StatementAST {
    //std::vector<std::string> ind_var_names;
    //std::string ind_var_name_; takes argument const std::string &ind_var_name;
    //BType ind_var_type_;
    std::unique_ptr<ExprAST> end_;
    std::unique_ptr<StatementAST> start_, step_, body_;
public:
    ForStatementAST(SourceLoc loc, std::unique_ptr<StatementAST> start, std::unique_ptr<ExprAST> end, std::unique_ptr<StatementAST> step, std::unique_ptr<StatementAST> body)
        : StatementAST(loc), start_(std::move(start)), end_(std::move(end)), step_(std::move(step)), body_(std::move(body)) {};
    llvm::Value *codegen() override;
    void accept(ASTVisitor * v) override {v->forStAction(this);};
};

class WhileStatementAST : public StatementAST {
    std::unique_ptr<ExprAST> cond_;
    std::unique_ptr<StatementAST> body_;
public:
    WhileStatementAST(SourceLoc loc, std::unique_ptr<ExprAST> cond, std::unique_ptr<StatementAST> body)
        : StatementAST(loc), cond_(std::move(cond)), body_(std::move(body)) {};
    llvm::Value *codegen() override;
    void accept(ASTVisitor * v) override {v->whileStAction(this);};
};

class ReturnStatementAST : public StatementAST {
    std::unique_ptr<ExprAST> return_expr_;
public:
    ReturnStatementAST(SourceLoc loc, std::unique_ptr<ExprAST> return_expr)
        : StatementAST(loc), return_expr_(std::move(return_expr)) {};
    llvm::Value *codegen() override;
    void accept(ASTVisitor * v) override {v->returnStAction(this);};
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
    void accept(ASTVisitor * v) override {v->blockStAction(this);};
};

class CallStatementAST : public StatementAST {
    std::unique_ptr<CallExprAST> call_;
public:
    CallStatementAST(SourceLoc loc, std::unique_ptr<CallExprAST> call)
        : StatementAST(loc), call_(std::move(call)) {};
    llvm::Value *codegen() override;
    void accept(ASTVisitor * v) override {v->callStAction(this);};
};

class AssignStatementAST : public StatementAST {
    std::string identifier_;
    std::unique_ptr<ExprAST> value_;
public:
    AssignStatementAST(SourceLoc loc, std::string identifier, std::unique_ptr<ExprAST> value)
        : StatementAST(loc), identifier_(identifier), value_(std::move(value)) {};
    llvm::Value *codegen() override;
    void accept(ASTVisitor * v) override {v->assignStAction(this);};
    std::string getIdentifier(){return identifier_;}
    std::shared_ptr<ExprAST> getValue(){return std::move(value_);};
};

class InitStatementAST : public StatementAST{
    std::string identifier_;
    BType var_type_;
    std::unique_ptr<AssignStatementAST> assignment_;
public:
    InitStatementAST(SourceLoc loc, std::string identifier, BType var_type, std::unique_ptr<AssignStatementAST> assignment)
        : StatementAST(loc), identifier_(identifier), var_type_(var_type), assignment_(std::move(assignment)) {};
    llvm::Value *codegen() override;
    void accept(ASTVisitor * v) override {v->initStAction(this);};
};

//-------------------------
// Function Expressions
//-------------------------

class PrototypeAST : public NodeAST{
    std::string name_;
    std::vector<std::pair<std::string,BType>> args_;
    BType return_type_;
public:
    PrototypeAST(SourceLoc loc, std::string name, std::vector<std::pair<std::string,BType>> args, BType return_type)
        : NodeAST(loc), name_(name), args_(args), return_type_(return_type) {};
    const std::string &getName() const {return name_;};
    llvm::Function *codegen();
    void accept(ASTVisitor * v) override {v->prototypeAction(this);};
};

class FunctionAST : public NodeAST{
    std::unique_ptr<PrototypeAST> proto_;
    std::unique_ptr<StatementAST> body_;
public:
    FunctionAST(SourceLoc loc, std::unique_ptr<PrototypeAST> proto, std::unique_ptr<StatementAST> body)
        : NodeAST(loc), proto_(std::move(proto)), body_(std::move(body)) {};
    llvm::Function *codegen();
    void accept(ASTVisitor * v) override {v->functionAction(this);};
};

} // namespace bassoon

#endif // Bassoon_include_ast_HXX