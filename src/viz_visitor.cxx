#include "viz_visitor.hxx"
#include "types.hxx"

namespace bassoon
{
namespace viz
{

VizVisitor::VizVisitor(){
    output_ = std::ofstream("../out/AST_Trees.dot", std::ofstream::out);
    node_base_names_ = std::set<std::string>({"Init","Bool","Int","Double","intType","boolType","doubleType","Var","Assign","Func","Proto","ProtoArg","ProtoRet","Block", "Binary"});
}

VizVisitor::~VizVisitor(){
    if(verbosity_){
        fprintf(stderr,"closing file\n");
    }
    output_ << std::endl;
    output_.close();
}

//---------------------
// Public driving functions
//---------------------

void VizVisitor::visualiseAST(std::shared_ptr<NodeAST> AST){
    output_ << "digraph { \n";
        AST->accept(this);
    output_ << "}";
}

void VizVisitor::visualiseASTs(std::vector<std::shared_ptr<NodeAST>> ASTs){
    
}

//-----------------------
// Node name management
//------------------------

std::string VizVisitor::getCurrentName(std::string s){
    std::set<std::string>::iterator it = node_base_names_.find(s);

    if(it == node_base_names_.end()){
        if(verbosity_) fprintf(stderr,"Node name not in known set, possible typo: %s\n", s.c_str());
    }
    int unique = unique_namer_map_[s];
    return s + std::to_string(unique);
}

void VizVisitor::advanceName(std::string s){
    unique_namer_map_[s]++;
}

std::string VizVisitor::getAndAdvanceName(std::string s){
    std::set<std::string>::iterator it = node_base_names_.find(s);

    if(it == node_base_names_.end()){
        if(verbosity_) fprintf(stderr,"Node name not in known set, possible typo: %s\n", s.c_str());
    }
    int unique = unique_namer_map_[s]++;
    return s + std::to_string(unique);
}

// ---------------------------
// .dot emission
// ---------------------------

void VizVisitor::addNodeLabel(std::string name, std::string label){
    output_ << name << "[label=\"" << label <<"\"]\n";
}

void VizVisitor::addNodeChild(std::string parent_name, std::string child_name){
    output_ << parent_name << "->" << child_name << "\n";
}

// -----------------------
// Name Stack management
// -----------------------

void VizVisitor::pushName(std::string s){
    name_stack_.push_back(s);
    if(verbosity_){
        fprintf(stderr, "-----pushed----\n");
        for (std::string s : name_stack_){
            fprintf(stderr,"%s\n",s.c_str());
        }
    }
}

std::string VizVisitor::popName(){
    std::string to_pop = name_stack_.at(name_stack_.size()-1);
    name_stack_.pop_back();
    if (verbosity_) {
        fprintf(stderr, "-----popped----\n");
        for (std::string s : name_stack_){
            fprintf(stderr,"%s\n",s.c_str());
        }
    }
    return to_pop;
}

//----------------------
// Expression actions
//----------------------

void VizVisitor::boolExprAction(BoolExprAST * bool_node) {
    // get the current name, emit node description, advance name
    std::string bool_name = getAndAdvanceName("Bool");
    pushName(bool_name);
    std::string value_string = bool_node->getValue()? "true": "false";
    addNodeLabel(bool_name, value_string);
}

void VizVisitor::intExprAction(IntExprAST * int_node) {
    std::string int_name = getAndAdvanceName("Int");
    pushName(int_name);
    std::string value_string = std::to_string(int_node->getValue());
    addNodeLabel(int_name, value_string);
}

void VizVisitor::doubleExprAction(DoubleExprAST * double_node) {
    std::string double_name = getAndAdvanceName("Double");
    pushName(double_name);
    std::string value_string = std::to_string(double_node->getValue());
    addNodeLabel(double_name, value_string);
}

void VizVisitor::variableExprAction(VariableExprAST * variable_node){
    std::string var_name = getAndAdvanceName("Var");
    pushName(var_name);
    std::string var_identifier = variable_node->getName();
    addNodeLabel(var_name, var_identifier);
}

void VizVisitor::callExprAction(CallExprAST * call_node) {
    std::string call_name = getAndAdvanceName("CallExpr");
    pushName(call_name);
    std::string call_identifier = call_node->getName();
    addNodeLabel(call_name, call_identifier+"()");
    // need to visit the arguments.

    std::shared_ptr<ExprAST> arg_node;
    std::string arg_name;
    while(call_node->anotherArg()){
        arg_node = call_node->getArg();
        arg_node->accept(this);
        arg_name = popName();
        addNodeChild(call_name, arg_name);
    }
}

void VizVisitor::unaryExprAction(UnaryExprAST * unary_node) {
    std::string unary_name = getAndAdvanceName("Unary");
    pushName(unary_name);
    std::string op_char = std::string(1, unary_node->getOpCode());
    addNodeLabel(unary_name, op_char);

    std::shared_ptr<ExprAST> operand_node = unary_node->getOperand();
    operand_node->accept(this);
    std::string operand_name = popName(); 

    addNodeChild(unary_name, operand_name);
}

void VizVisitor::binaryExprAction(BinaryExprAST * binary_node) {
    std::string binary_name = getAndAdvanceName("Binary");
    pushName(binary_name);
    std::string op_char = std::string(1, binary_node->getOpCode());
    addNodeLabel(binary_name, op_char);

    std::shared_ptr<ExprAST> lhs = binary_node->getLHS();
    lhs->accept(this);
    std::string lhs_name = popName();
    addNodeChild(binary_name, lhs_name);

    std::shared_ptr<ExprAST> rhs = binary_node->getRHS();
    rhs->accept(this);
    std::string rhs_name = popName();
    addNodeChild(binary_name, rhs_name);
}

// ------------------
// Statement Actions
// ------------------

void VizVisitor::ifStAction(IfStatementAST * if_node) {
    std::string if_name = getAndAdvanceName("If");
    addNodeLabel(if_name,"if");
    pushName(if_name);
    
    std::shared_ptr<ExprAST> cond_node = if_node->getCond();
    cond_node->accept(this);
    std::string cond_name = popName();
    addNodeChild(if_name, cond_name);

    std::shared_ptr<StatementAST> then_node = if_node->getThen();
    then_node->accept(this);
    std::string then_name = popName();
    addNodeChild(if_name, then_name);

    std::shared_ptr<StatementAST> else_node = if_node->getElse();
    else_node->accept(this);
    std::string else_name = popName();
    addNodeChild(if_name, else_name);
}

void VizVisitor::forStAction(ForStatementAST * for_node) {
    std::string for_name = getAndAdvanceName("For");
    pushName(for_name);
    addNodeLabel(for_name, "for(){...}");

    std::shared_ptr<StatementAST> start_node = for_node->getStart();
    start_node->accept(this);
    std::string start_name = popName();
    addNodeChild(for_name, start_name);

    std::shared_ptr<ExprAST> cond_node = for_node->getEnd();
    cond_node->accept(this);
    std::string cond_name = popName();
    addNodeChild(for_name, cond_name);

    std::shared_ptr<StatementAST> step_node = for_node->getStep();
    step_node->accept(this);
    std::string step_name = popName();
    addNodeChild(for_name, step_name);

    std::shared_ptr<StatementAST> body_node = for_node->getBody();
    body_node->accept(this);
    std::string body_name = popName();
    addNodeChild(for_name, body_name);
}

void VizVisitor::whileStAction(WhileStatementAST * while_node) {
    std::string while_name = getAndAdvanceName("While");
    pushName(while_name);
    addNodeLabel(while_name, "while(){...}");

    std::shared_ptr<ExprAST> cond_node = while_node->getCond();
    cond_node->accept(this);
    std::string cond_name = popName();
    addNodeChild(while_name, cond_name);

    std::shared_ptr<StatementAST> body_node = while_node->getBody();
    body_node->accept(this);
    std::string body_name = popName();
    addNodeChild(while_name, body_name);
}

void VizVisitor::returnStAction(ReturnStatementAST * return_node) {
    std::string return_name = getAndAdvanceName("Return");
    addNodeLabel(return_name,"return");
    pushName(return_name);

    std::shared_ptr<ExprAST> expr_node = return_node->getReturnExpr();
    expr_node->accept(this);
    std::string expr_name = popName();
    addNodeChild(return_name, expr_name);
}

void VizVisitor::blockStAction(BlockStatementAST * block_node) {
    std::string block_name = getAndAdvanceName("Block");
    addNodeLabel(block_name,"{...}");
    pushName(block_name);

    std::shared_ptr<StatementAST> statement_node;
    std::string statement_name;
    block_node->resetStatementIndex();
    //fprintf(stderr,"action %i\n",block_node->anotherStatement());
    while (block_node->anotherStatement()){
        statement_node = block_node->getStatement();
        statement_node->accept(this);
        statement_name = popName();
        addNodeChild(block_name,statement_name);
    }
}

void VizVisitor::callStAction(CallStatementAST * call_node) {
    std::string call_name = getAndAdvanceName("CallSt");
    pushName(call_name);
    addNodeLabel(call_name,"call");
    
    std::shared_ptr<CallExprAST> call_expr_node = call_node->getCall();
    call_expr_node->accept(this);
    std::string call_expr_name = popName();

    addNodeChild(call_name,call_expr_name);
}

void VizVisitor::assignStAction(AssignStatementAST * assign_node) {
    std::string assign_name = getAndAdvanceName("Assign");
    pushName(assign_name);
    std::string var_string = assign_node->getIdentifier();
    std::string var_name = getAndAdvanceName("Var");
    addNodeLabel(assign_name, "=");
    addNodeLabel(var_name,var_string);
    addNodeChild(assign_name, var_name);

    std::shared_ptr<ExprAST> val_expr_node = assign_node->getValue();
    val_expr_node->accept(this);
    std::string val_expr_name = popName();
    addNodeChild(assign_name, val_expr_name);
}

void VizVisitor::initStAction(InitStatementAST * init_node) {
    std::string init_name = getAndAdvanceName("Init");
    pushName(init_name);
    std::string var_string = init_node->getIdentifier();
    std::string var_name = getAndAdvanceName("Var");
    addNodeLabel(init_name, "init");
    addNodeLabel(var_name,var_string);
    addNodeChild(init_name, var_name);

    BType var_type = init_node->getType();
    // typeToStr returns "bool", "int", "double"...
    std::string type_str = typeToStr(var_type);
    std::string type_node_name = getAndAdvanceName(type_str+"Type");
    addNodeLabel(type_node_name, type_str);
    addNodeChild(init_name, type_node_name);

    std::shared_ptr<AssignStatementAST> assign_node = init_node->getAssignment();
    assign_node->accept(this);
    std::string assign_node_name = popName();
    addNodeChild(init_name, assign_node_name);
}

//-----------------
// Miscellaneous Actions
//-----------------

void VizVisitor::prototypeAction(PrototypeAST * proto_node) {
    std::string proto_name = getAndAdvanceName("Proto");
    pushName(proto_name);
    std::string func_name = proto_node->getName();
    std::vector<std::pair<std::string, BType>> proto_args = proto_node->getArgs();
    addNodeLabel(proto_name, func_name);

    std::string arg_name, arg_str;
    for(auto pair : proto_args){
        arg_name = getAndAdvanceName("ProtoArg");
        arg_str = pair.first + ": " +typeToStr(pair.second);
        addNodeLabel(arg_name, arg_str);
        addNodeChild(proto_name, arg_name);
    }
    BType ret_type = proto_node->getRetType();
    if (ret_type != type_void){
        std::string ret_name = getAndAdvanceName("ProtoRet");
        std::string ret_str = "return: " + typeToStr(proto_node->getRetType());
        addNodeLabel(ret_name, ret_str);
        addNodeChild(proto_name, ret_name);
    }
}

void VizVisitor::functionAction(FunctionAST * func_node) {
    std::string function_name = getAndAdvanceName("Func");

    std::shared_ptr<PrototypeAST> proto_node = func_node->getProto();
    std::string identifier_str = proto_node->getName();
    addNodeLabel(function_name, "define "+identifier_str);

    proto_node->accept(this);
    std::string proto_name = popName();
    addNodeChild(function_name, proto_name);

    std::shared_ptr<StatementAST> body_node = func_node->getBody();
    body_node->accept(this);
    std::string body_name = popName();
    addNodeChild(function_name, body_name);
}

} // namespace viz

} // namespace bassoon
