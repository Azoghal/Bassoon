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
class TopLevels;
class FuncDefs;
class BProgram ;

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

    virtual void topLevelsAction(TopLevels * top_levels_node) = 0;
    virtual void funcDefsAction(FuncDefs * func_defs_node) = 0;
    virtual void programAction(BProgram * program_node) = 0;
};

class NodeAST{
public:
    virtual ~NodeAST() = default;
    virtual void accept(ASTVisitor * v) = 0;
};

class SrcNodeAST:NodeAST{
    SourceLoc loc_;
public:
    SrcNodeAST(SourceLoc loc) : loc_(loc) {};
    virtual ~SrcNodeAST() = default;
    int getLine() const {return loc_.line;};
    int getCol() const {return loc_.collumn;};
    std::string getLocStr() const{
        std::string formatted_loc = "{" + std::to_string(loc_.line) + ":" + std::to_string(loc_.collumn) + "}";
        return formatted_loc;
    }
};

//----------------------
// Base Expression class
//----------------------

class ExprAST : public SrcNodeAST{
    BType type_;
public:
    ExprAST(SourceLoc loc, BType type) : SrcNodeAST(loc), type_(type) {};
    ExprAST(SourceLoc loc) : SrcNodeAST(loc), type_(type_unknown) {};
    virtual ~ExprAST() = default;
    void accept(ASTVisitor * v) override {}
    const BType & getType() const {return type_;}
    void setType(BType known_type){
        // >= 0 excludes void so for CallExpr this is overrided
        if (type_ == type_unknown && known_type>=0){
            type_ = known_type;
        } else{
            if(type_!=known_type){
                fprintf(stderr,"Tried to overwrite known type %s with %s at %s\n",typeToStr(type_).c_str(), typeToStr(known_type).c_str(), getLocStr().c_str());
            }
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
    int getValue() const {return value_;}
};

class DoubleExprAST : public ValueExprAST{
    double value_;
public:
    DoubleExprAST(SourceLoc loc, double value) 
        : ValueExprAST(loc, type_double), value_(value) {};
    void accept(ASTVisitor * v) override {v->doubleExprAction(this);}
    double getValue() const {return value_;}
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
    void argAcceptOne(ASTVisitor * v){args_[arg_index_++]->accept(v);}
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

class StatementAST : public SrcNodeAST{
public:
    StatementAST(SourceLoc loc) : SrcNodeAST(loc) {};
    virtual ~StatementAST() = default;
    void accept(ASTVisitor * v) override {};
};

//--------------------------
// Control Flow Statements
//--------------------------

class IfStatementAST : public StatementAST {
    std::unique_ptr<ExprAST> cond_;
    std::unique_ptr<StatementAST> then_, else_;
    //std::vector<std::unique_ptr<StatementAST>> elseifs?
    bool has_else_;
public:
    IfStatementAST(SourceLoc loc, std::unique_ptr<ExprAST> cond, std::unique_ptr<StatementAST> then, std::unique_ptr<StatementAST> elsewise, bool has_else) 
        : StatementAST(loc), cond_(std::move(cond)), then_(std::move(then)), else_(std::move(elsewise)), has_else_(has_else) {};
    void accept(ASTVisitor * v) override {v->ifStAction(this);};
    const ExprAST & getCond() const {return  *cond_;}
    const StatementAST & getThen() const {return  *then_;}
    const StatementAST & getElse() const {return  *else_;}
    void condAccept(ASTVisitor * v){cond_->accept(v);}
    void thenAccept(ASTVisitor * v){then_->accept(v);}
    void elseAccept(ASTVisitor * v){if(!has_else_){fprintf(stderr,"don't have an else clause\n");};else_->accept(v);}
    bool getHasElse(){return has_else_;}
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
    // void statementAcceptAll(ASTVisitor * v){
    //     resetStatementIndex();
    //     while(anotherStatement()){
    //         statementAcceptOne(v);
    //     }
    // }
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
    const AssignStatementAST &getAssignment() const {return *assignment_;}
    void assignmentAccept(ASTVisitor * v) {assignment_->accept(v);}
};

//-------------------------
// Function Expressions
//-------------------------

class PrototypeAST : public SrcNodeAST{
    std::string name_;
    std::vector<std::pair<std::string,BType>> args_;
    BFType func_type_;
public:
    PrototypeAST(SourceLoc loc, std::string name, std::vector<std::pair<std::string,BType>> args, BFType func_type)
        : SrcNodeAST(loc), name_(name), args_(args), func_type_(func_type) {};
    const std::string &getName() const {return name_;};
    void accept(ASTVisitor * v) override {v->prototypeAction(this);};
    const std::vector<std::pair<std::string,BType>> & getArgs(){return args_;};
    const BType & getRetType(){return func_type_.getReturnType();}
    const BFType & getType(){return func_type_;}
};

class FunctionAST : public SrcNodeAST{
    std::unique_ptr<PrototypeAST> proto_;
    std::unique_ptr<StatementAST> body_;
public:
    FunctionAST(SourceLoc loc, std::unique_ptr<PrototypeAST> proto, std::unique_ptr<StatementAST> body)
        : SrcNodeAST(loc), proto_(std::move(proto)), body_(std::move(body)) {};
    void accept(ASTVisitor * v) override {v->functionAction(this);};
    const PrototypeAST & getProto() const {return *proto_;};
    const StatementAST & getBody() const {return *body_;};
    const BFType & getType() const {return proto_->getType();}
    void protoAccept(ASTVisitor * v){proto_->accept(v);}
    void bodyAccept(ASTVisitor * v){body_->accept(v);}
};

// ---------------
//   AST grouping
// ---------------

// class BGlobals : public SrcNodeAST{} // holds any 'global' statements in top level

// Function definitions
class FuncDefs : public NodeAST {
    std::vector<std::unique_ptr<FunctionAST>> func_ASTs_;
    //std::vector<std::unique_ptr<FunctionAST>>::iterator func_iter;
    int func_index_ = 0;
public:
    FuncDefs(std::vector<std::unique_ptr<FunctionAST>> func_ASTs)
        : func_ASTs_(std::move(func_ASTs)) {}
    void accept(ASTVisitor * v) override {v->funcDefsAction(this);};
    void addFunction(std::unique_ptr<FunctionAST> func_AST){func_ASTs_.push_back(std::move(func_AST));}
    bool anotherFunc(){return func_index_<func_ASTs_.size();}
    void functionsAllAccept(ASTVisitor * v) {
        while(anotherFunc()){
            fprintf(stderr,"function %s\n", func_ASTs_[func_index_]->getProto().getName().c_str());
            func_ASTs_[func_index_]->accept(v);
            func_index_++;
        }
        func_index_=0;
    }
    int countFuncs() const {return func_ASTs_.size();}
    //std::vector<std::unique_ptr<FunctionAST>>::iterator getFuncASTsIter(){return func_iter.begin();};
};

// Top Level statement ASTs
class TopLevels : public NodeAST {
    std::vector<std::unique_ptr<StatementAST>> statement_ASTs_;
    int statement_index_ =0;
public:
    TopLevels(std::vector<std::unique_ptr<StatementAST>> statement_ASTs)
        : statement_ASTs_(std::move(statement_ASTs)) {}
    void accept(ASTVisitor * v) override {v->topLevelsAction(this);};
    void addStatement(std::unique_ptr<StatementAST> statement_AST){statement_ASTs_.push_back(std::move(statement_AST));}
    bool anotherStatement(){return statement_index_<statement_ASTs_.size();}
    void statementsAllAccept(ASTVisitor * v) {
        while(anotherStatement()){
            fprintf(stderr,"statement %d\n", statement_index_);
            statement_ASTs_[statement_index_]->accept(v);
            statement_index_++;
        }
        statement_index_=0;
    }
    int countStatements() const {return statement_ASTs_.size();}
};

// Overall Program
class BProgram : public NodeAST {
    std::unique_ptr<TopLevels> top_levels_; 
    std::unique_ptr<FuncDefs> func_defs_;
public:
    BProgram(std::unique_ptr<TopLevels> top_levels, std::unique_ptr<FuncDefs> func_defs) 
        : func_defs_(std::move(func_defs)), top_levels_(std::move(top_levels)) {};
    void accept(ASTVisitor * v) override {v->programAction(this);};
    const TopLevels & getTopLevels() const {return *top_levels_;};
    const FuncDefs & getFuncDefs() const {return *func_defs_;};
    void topLevelsAccept(ASTVisitor * v){top_levels_->accept(v);}
    void funcDefsAccept(ASTVisitor * v){func_defs_->accept(v);}
};

} // namespace bassoon

#endif // Bassoon_include_ast_HXX