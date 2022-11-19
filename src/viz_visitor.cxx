#include "viz_visitor.hxx"

namespace bassoon
{
namespace viz
{

VizVisitor::VizVisitor(){
    output_ = std::ofstream("../out/AST_Trees.dot", std::ofstream::out);
    node_base_names_ = std::set<std::string>({"Init","Bool","Int","Double"});
}

VizVisitor::~VizVisitor(){
    fprintf(stderr,"closing file\n");
    output_ << std::endl;
    output_.close();
}

//---------------------
// Public driving functions
//---------------------

void VizVisitor::visualiseAST(std::shared_ptr<NodeAST> AST){
    output_ << "digraph { \n";
        output_ << "A->B\n";
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
        fprintf(stderr,"Node name not in known set, possible typo: %s\n", s.c_str());
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
        fprintf(stderr,"Node name not in known set, possible typo: %s\n", s.c_str());
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
}

std::string VizVisitor::popName(std::string s){
    std::string to_pop = name_stack_.at(name_stack_.size()-1);
    if (to_pop != s){
        fprintf(stderr,"Name popped off (%s) did not match passed name (%s)", to_pop.c_str(), s.c_str());
    }
    name_stack_.pop_back();
    return to_pop;
}

std::string VizVisitor::popName(){
    std::string to_pop = name_stack_.at(name_stack_.size()-1);
    name_stack_.pop_back();
    return to_pop;
}

//----------------------
// Expression actions
//----------------------

void VizVisitor::boolExprAction(BoolExprAST * bool_node) {
    // get the current name, emit node description, advance name
    std::string unique_name = getAndAdvanceName("Bool");
    pushName(unique_name);
    std::string value_string = bool_node->getValue()? "true": "false";
    addNodeLabel(unique_name, value_string);
}

void VizVisitor::intExprAction(IntExprAST * int_node) {
    std::string unique_name = getAndAdvanceName("Int");
    pushName(unique_name);
    std::string value_string = std::to_string(int_node->getValue());
    addNodeLabel(unique_name, value_string);
}

void VizVisitor::doubleExprAction(DoubleExprAST * double_node) {
    std::string unique_name = getAndAdvanceName("Double");
    pushName(unique_name);
    std::string value_string = std::to_string(double_node->getValue());
    addNodeLabel(unique_name, value_string);
}

void VizVisitor::variableExprAction(VariableExprAST * variable_node){
    std::string unique_name = getAndAdvanceName("Var");
    pushName(unique_name);
    std::string var_name = variable_node->getName();
    addNodeLabel(unique_name, var_name);
}

void VizVisitor::callExprAction(CallExprAST * call_node) {
    std::string unique_name = getAndAdvanceName("CallExpr");
    pushName(unique_name);
    std::string call_name = call_node->getName();
    addNodeLabel(unique_name, call_name);
    // need to visit the arguments.
}

void VizVisitor::unaryExprAction(UnaryExprAST * unary_node) {
    std::string unique_name = getAndAdvanceName("Unary");
    pushName(unique_name);
    std::string op_char = std::to_string(unary_node->getOpCode());
    addNodeLabel(unique_name, op_char);

    std::shared_ptr<ExprAST> operand_node = unary_node->getOperand();
    operand_node->accept(this);
    std::string operand_name = popName(); 

    addNodeChild(unique_name, operand_name);
}

void VizVisitor::binaryExprAction(BinaryExprAST * binary_node) {
    std::string unique_name = getAndAdvanceName("Binary");
    pushName(unique_name);
    std::string op_char = std::to_string(binary_node->getOpCode());
    addNodeLabel(unique_name, op_char);

    std::shared_ptr<ExprAST> lhs = binary_node->getLHS();
    lhs->accept(this);
    std::string lhs_name = popName();
    addNodeChild(unique_name, lhs_name);

    std::shared_ptr<ExprAST> rhs = binary_node->getRHS();
    rhs->accept(this);
    std::string rhs_name = popName();
    addNodeChild(unique_name, rhs_name);
}

// ------------------
// Statement Actions
// ------------------

void VizVisitor::ifStAction(IfStatementAST * if_node) {}
void VizVisitor::forStAction(ForStatementAST * for_node) {}
void VizVisitor::whileStAction(WhileStatementAST * while_node) {}
void VizVisitor::returnStAction(ReturnStatementAST * return_node) {}
void VizVisitor::blockStAction(BlockStatementAST * block_node) {}
void VizVisitor::callStAction(CallStatementAST * call_node) {}

void VizVisitor::assignStAction(AssignStatementAST * assign_node) {
    std::string unique_name = getAndAdvanceName("Assign");
    std::string var_string = assign_node->getIdentifier();
    std::string var_name = getAndAdvanceName("Var");
    addNodeLabel(unique_name, "=");
    addNodeLabel(var_name,var_string);
    addNodeChild(unique_name, var_name);

    std::shared_ptr<ExprAST> val_expr_node = assign_node->getValue();
    val_expr_node->accept(this);
    std::string val_expr_name = popName();
    addNodeChild(unique_name, val_expr_name);
}

void VizVisitor::initStAction(InitStatementAST * init_node) {}

//-----------------
// Miscellaneous Actions
//-----------------

void VizVisitor::prototypeAction(PrototypeAST * proto_node) {}
void VizVisitor::functionAction(FunctionAST * func_node) {}

} // namespace viz

} // namespace bassoon
