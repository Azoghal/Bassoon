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
    std::string getLocStr(){
        std::string formatted_loc = "{" + std::to_string(loc_.line) + ":" + std::to_string(loc_.collumn) + "}";
        return formatted_loc;
    }
};

//----------------------
// Base Expression class
//----------------------

class ExprAST : public NodeAST{
    BType type_;
public:
    ExprAST(SourceLoc loc, BType type) : NodeAST(loc), type_(type) {};
    ExprAST(SourceLoc loc) : NodeAST(loc), type_(type_unknown) {};
    virtual ~ExprAST() = default;
    void accept(ASTVisitor * v) override {}
    const BType & getType() const {return type_;}
    void setType(BType known_type){
        if (type_ == type_unknown && known_type >= 0){// >= 0 includes void...
            type_ = known_type;
        } else{
            fprintf(stderr,"Tried to overwrite known type\n");
        }
    }
};

//----------------------
// Value Expressions
//----------------------

class ValueExprAST : public ExprAST {
    // REDUNDANT
public: 
    ValueExprAST(SourceLoc loc, BType type)
        : ExprAST(loc, type) {};
};

class BoolExprAST : public ValueExprAST {
    bool value_;
public:
    BoolExprAST(SourceLoc loc, bool value) 
        : ValueExprAST(loc, type_bool), value_(value) {};
    void accept(ASTVisitor * v) override {v->boolExprAction(this);}
    bool getValue() const {return value_;}
};

class IntExprAST : public ValueExprAST{
    int value_;
public:
    IntExprAST(SourceLoc loc, int value) 
        : ValueExprAST(loc, type_int), value_(value) {};
    void accept(ASTVisitor * v) override {v->intExprAction(this);}
    bool getValue() const {return value_;}
};

class DoubleExprAST : public ValueExprAST{
    double value_;
public:
    DoubleExprAST(SourceLoc loc, double value) 
        : ValueExprAST(loc, type_double), value_(value) {};
    void accept(ASTVisitor * v) override {v->doubleExprAction(this);}
    bool getValue() const {return value_;}
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
    void accept(ASTVisitor * v) override {v->variableExprAction(this);}
    const std::string getName() const {return name_;};
};

class CallExprAST : public ExprAST {
    std::string callee_;
    std::vector<std::unique_ptr<ExprAST>> args_;
    int arg_index_ = 0;
public:
    CallExprAST(SourceLoc loc, const std::string &callee, std::vector<std::unique_ptr<ExprAST>> args) 
        : ExprAST(loc), callee_(callee), args_(std::move(args)) {};
    void accept(ASTVisitor * v) override {v->callExprAction(this);};
    const std::string getName() const {return callee_;}
    void resetArgIndex(){arg_index_ = 0;}
    bool anotherArg() const {return arg_index_ < args_.size();}
    const ExprAST & getOneArg(){return *args_[arg_index_++];}
    void argAcceptOne(ASTVisitor * v){args_[arg_index_++];}
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
    void accept(ASTVisitor * v) override {v->unaryExprAction(this);};
    char getOpCode(){return  opcode_;}
    const ExprAST & getOperand() const {return *operand_;}
    void operandAccept(ASTVisitor * v) {operand_->accept(v);}
};

class BinaryExprAST : public ExprAST {
    char opcode_;
    std::unique_ptr<ExprAST> lhs_, rhs_;
public:
    BinaryExprAST(SourceLoc loc, char opcode, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs)
        : ExprAST(loc), opcode_(opcode), lhs_(std::move(lhs)), rhs_(std::move(rhs)) {};
    void accept(ASTVisitor * v) override {v->binaryExprAction(this);};
    const char getOpCode() const {return opcode_;};
    const ExprAST & getLHS() const {return *lhs_;}
    const ExprAST & getRHS() const {return *rhs_;}
    void lhsAccept(ASTVisitor * v) {lhs_->accept(v);}
    void rhsAccept(ASTVisitor * v) {rhs_->accept(v);}
};
// -------------------------
// Statements
// -------------------------

class StatementAST : public NodeAST{
public:
    StatementAST(SourceLoc loc) : NodeAST(loc) {};
    virtual ~StatementAST() = default;
    void accept(ASTVisitor * v) override {};
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
    void accept(ASTVisitor * v) override {v->ifStAction(this);};
    const ExprAST & getCond() const {return  *cond_;}
    const StatementAST & getThen() const {return  *then_;}
    const StatementAST & getElse() const {return  *else_;}
    void condAccept(ASTVisitor * v){cond_->accept(v);}
    void thenAccept(ASTVisitor * v){then_->accept(v);}
    void elseAccept(ASTVisitor * v){else_->accept(v);}
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
    void accept(ASTVisitor * v) override {v->forStAction(this);};
    const ExprAST & getEnd() const {return  *end_;}
    const StatementAST & getStart() const {return  *start_;}
    const StatementAST & getStep() const {return  *step_;}
    const StatementAST & getBody() const {return  *body_;}
    void endAccept(ASTVisitor * v){end_->accept(v);}
    void startAccept(ASTVisitor * v){start_->accept(v);}
    void stepAccept(ASTVisitor * v){step_->accept(v);}
    void bodyAccept(ASTVisitor * v){body_->accept(v);}
};

class WhileStatementAST : public StatementAST {
    std::unique_ptr<ExprAST> cond_;
    std::unique_ptr<StatementAST> body_;
public:
    WhileStatementAST(SourceLoc loc, std::unique_ptr<ExprAST> cond, std::unique_ptr<StatementAST> body)
        : StatementAST(loc), cond_(std::move(cond)), body_(std::move(body)) {};
    void accept(ASTVisitor * v) override {v->whileStAction(this);};
    const ExprAST & getCond() const {return *cond_;}
    const StatementAST & getBody() const {return *body_;};
    void condAccept(ASTVisitor * v) {cond_->accept(v);}
    void bodyAccept(ASTVisitor * v) {body_->accept(v);}
};


class ReturnStatementAST : public StatementAST {
    std::unique_ptr<ExprAST> return_expr_;
    BType return_type_;
public:
    ReturnStatementAST(SourceLoc loc, std::unique_ptr<ExprAST> return_expr)
        : StatementAST(loc), return_expr_(std::move(return_expr)) {};
    void accept(ASTVisitor * v) override {v->returnStAction(this);};
    const ExprAST & getReturnExpr() const {return *return_expr_;}
    void returnExprAccept(ASTVisitor * v) {return_expr_->accept(v);}
    BType getReturnType() const {return return_type_;}
    void setReturnType(BType ret_type) {return_type_ = ret_type;}
};

// ------------------------
// Other Statements
// -----------------------

class BlockStatementAST : public StatementAST{
    std::vector<std::unique_ptr<StatementAST>> statements_;
    int statement_index_ = 0;
    bool has_return_;
    BType return_type_;
public:
    BlockStatementAST(SourceLoc loc, std::vector<std::unique_ptr<StatementAST>> statements)
        : StatementAST(loc), statements_(std::move(statements)) {};
    void accept(ASTVisitor * v) override {v->blockStAction(this);};
    void resetStatementIndex(){statement_index_=0;}
    bool anotherStatement(){return statement_index_ < statements_.size();};
    const StatementAST & getOneStatement() {return *statements_[statement_index_++];}
    void statementAcceptOne(ASTVisitor *  v){statements_[statement_index_++]->accept(v);}
    bool hasReturn() const {return has_return_;}
    BType getReturnType() const {return return_type_;}
};

class CallStatementAST : public StatementAST {
    std::unique_ptr<CallExprAST> call_;
public:
    CallStatementAST(SourceLoc loc, std::unique_ptr<CallExprAST> call)
        : StatementAST(loc), call_(std::move(call)) {};
    void accept(ASTVisitor * v) override {v->callStAction(this);};
    const CallExprAST & getCall() const {return *call_;}
    void callAccept(ASTVisitor * v) {call_->accept(v);}
};

class AssignStatementAST : public StatementAST {
    std::string identifier_;
    std::unique_ptr<ExprAST> value_;
public:
    AssignStatementAST(SourceLoc loc, std::string identifier, std::unique_ptr<ExprAST> value)
        : StatementAST(loc), identifier_(identifier), value_(std::move(value)) {};
    void accept(ASTVisitor * v) override {v->assignStAction(this);};
    const std::string getIdentifier() const {return identifier_;}
    const ExprAST & getValue() const {return *value_;};
    void valueAccept(ASTVisitor * v){value_->accept(v);}
};

class InitStatementAST : public StatementAST{
    std::string identifier_;
    BType var_type_;
    std::unique_ptr<AssignStatementAST> assignment_;
public:
    InitStatementAST(SourceLoc loc, std::string identifier, BType var_type, std::unique_ptr<AssignStatementAST> assignment)
        : StatementAST(loc), identifier_(identifier), var_type_(var_type), assignment_(std::move(assignment)) {};
    void accept(ASTVisitor * v) override {v->initStAction(this);};
    const std::string getIdentifier() const {return identifier_;}
    const BType getType() const {return var_type_;}
    const AssignStatementAST &getAssignment() const {if(!assignment_){fprintf(stderr,"seems we've thrown the assignment away");};return *assignment_;}
    void assignmentAccept(ASTVisitor * v) {assignment_->accept(v);}
};

//-------------------------
// Function Expressions
//-------------------------

class PrototypeAST : public NodeAST{
    std::string name_;
    std::vector<std::pair<std::string,BType>> args_;
    BFType func_type_;
public:
    PrototypeAST(SourceLoc loc, std::string name, std::vector<std::pair<std::string,BType>> args, BFType func_type)
        : NodeAST(loc), name_(name), args_(args), func_type_(func_type) {};
    const std::string &getName() const {return name_;};
    void accept(ASTVisitor * v) override {v->prototypeAction(this);};
    const std::vector<std::pair<std::string,BType>> & getArgs(){return args_;};
    const BType & getRetType(){return func_type_.getReturnType();}
    const BFType & getType(){return func_type_;}
};

class FunctionAST : public NodeAST{
    std::unique_ptr<PrototypeAST> proto_;
    std::unique_ptr<StatementAST> body_;
public:
    FunctionAST(SourceLoc loc, std::unique_ptr<PrototypeAST> proto, std::unique_ptr<StatementAST> body)
        : NodeAST(loc), proto_(std::move(proto)), body_(std::move(body)) {};
    void accept(ASTVisitor * v) override {v->functionAction(this);};
    const PrototypeAST & getProto() const {return *proto_;};
    const StatementAST & getBody() const {return *body_;};
    const BFType & getType() const {return proto_->getType();}
    void protoAccept(ASTVisitor * v){proto_->accept(v);}
    void bodyAccept(ASTVisitor * v){body_->accept(v);}
};

} // namespace bassoon

#endif // Bassoon_include_ast_HXX