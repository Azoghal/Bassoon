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
    output_.close();
}

void VizVisitor::visualiseAST(std::shared_ptr<NodeAST> AST){
    output_ << "digraph { \n";
        output_ << "A->B\n";
    output_ << "}";
}

void VizVisitor::visualiseASTs(std::vector<std::shared_ptr<NodeAST>> ASTs){
    
}

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

void VizVisitor::addNodeLabel(std::string name, std::string label){
    output_ << name << "[label=\"" << label <<"\"]\n";
}

void VizVisitor::boolExprAction(BoolExprAST * bool_node) {
    // get the current name, emit node description, advance name
    std::string unique_name = getAndAdvanceName("Bool");
    std::string value_string = bool_node->getValue()? "true": "false";
    addNodeLabel(unique_name, value_string);
}

void VizVisitor::intExprAction(IntExprAST * int_node) {
    std::string unique_name = getAndAdvanceName("Int");
    std::string value_string = std::to_string(int_node->getValue());
    addNodeLabel(unique_name, value_string);
}

void VizVisitor::doubleExprAction(DoubleExprAST * double_node) {
    std::string unique_name = getAndAdvanceName("Double");
    std::string value_string = std::to_string(double_node->getValue());
    addNodeLabel(unique_name, value_string);
}

void VizVisitor::variableExprAction(VariableExprAST * variable_node) {
    std::string unique_name = getAndAdvanceName("Var");
    std::string var_name = variable_node->getName();
    addNodeLabel(unique_name, var_name);
}

void VizVisitor::callExprAction(CallExprAST * call_node) {
    std::string unique_name = getAndAdvanceName("CallExpr");
    std::string call_name = call_node->getName();
    addNodeLabel(unique_name, call_name);
    // need to visit the arguments.
}

void VizVisitor::unaryExprAction(UnaryExprAST * unary_node) {
    std::string unique_name = getAndAdvanceName("Var");
    std::string op_char = std::to_string(unary_node->getOpCode());
    addNodeLabel(unique_name, op_char);
    // need to visit child, and find the name - from stack.
}
void VizVisitor::binaryExprAction(BinaryExprAST * binary_node) {}

void VizVisitor::ifStAction(IfStatementAST * if_node) {}
void VizVisitor::forStAction(ForStatementAST * for_node) {}
void VizVisitor::whileStAction(WhileStatementAST * while_node) {}
void VizVisitor::returnStAction(ReturnStatementAST * return_node) {}
void VizVisitor::blockStAction(BlockStatementAST * block_node) {}
void VizVisitor::callStAction(CallStatementAST * call_node) {}
void VizVisitor::assignStAction(AssignStatementAST * assign_node) {}
void VizVisitor::initStAction(InitStatementAST * init_node) {}

void VizVisitor::prototypeAction(PrototypeAST * proto_node) {}
void VizVisitor::functionAction(FunctionAST * func_node) {}

} // namespace viz

} // namespace bassoon
