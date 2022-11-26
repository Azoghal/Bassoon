#include "type_visitor.hxx"
#include "types.hxx"
#include "inbuilt_operators.hxx"

namespace bassoon
{
namespace typecheck
{

void typingMessage(std::string message){
    fprintf(stderr, "%s", message.c_str());
}

void typingMessage(std::string message, std::string var_or_func_name){
    fprintf(stderr, "%s: %s", message.c_str(),var_or_func_name.c_str());
}

void typingMessage(std::string message, std::string var_or_func_name, std::string loc_str){
    fprintf(stderr, "%s: %s at %s", message.c_str(), var_or_func_name.c_str(), loc_str.c_str());
}

BType TypeVisitor::typeContext(std::string identifier){
    std::vector<BType> identifier_types = identifier_stacks_[identifier];
    if (identifier_types.size() == 0){
        typingMessage("Variable referenced before definition", identifier);
        // throw?
    }
    return identifier_types[identifier_types.size()-1];
}

BFType TypeVisitor::funcContext(std::string func_name){
    BFType func_type = func_types_[func_name];
    if (!func_type.isValid()){
        typingMessage("Function not defined", func_name);
        // throw?
    }
    return func_type;
}

bool TypeVisitor::varIsDefined(std::string identifier){
    std::vector<BType> identifier_types = identifier_stacks_[identifier];
    if (identifier_types.size() == 0){
        return false;
    }
    return true;
}

bool TypeVisitor::funcIsDefined(std::string func_name){
    BFType func_type = func_types_[func_name];
    return func_type.isValid();
}

// bool hasType(ExprAST * node){
//     if (node->getType() == type_unknown){
//         return false;
//     }
//     return true;
// }

bool hasType(std::shared_ptr<ExprAST> node){
    if (node->getType() == type_unknown){
        return false;
    }
    return true;
}

// Expression actions should ensure that sub-expressions are visited and populated with types
// and that the types match any requirements

void TypeVisitor::boolExprAction(BoolExprAST * bool_node) {
    if (bool_node->getType() == type_unknown){
        typingMessage("bool expression without known type", "", bool_node->getLocStr());
    } 
}

void TypeVisitor::intExprAction(IntExprAST * int_node) {
    if (int_node->getType() == type_unknown){
        typingMessage("int expression without known type", "", int_node->getLocStr());
    } 
}
void TypeVisitor::doubleExprAction(DoubleExprAST * double_node) {
    if (double_node->getType() == type_unknown){
        typingMessage("double expression without known type", "", double_node->getLocStr());
    } 
}

void TypeVisitor::variableExprAction(VariableExprAST * variable_node) {
    // Variables should only appear when they are defined
    std::string variable_name = variable_node->getName();
    if (!varIsDefined(variable_name)){
        std::string loc_str = variable_node->getLocStr();
        typingMessage("Variable not defined before use", variable_name, loc_str);
        // throw
    }else{
        variable_node->setType(typeContext(variable_name));
    }
}

void TypeVisitor::callExprAction(CallExprAST * call_node) {
    std::string func_name = call_node->getName();
    if(!funcIsDefined(func_name)){
        std::string loc_str = call_node->getLocStr();
        typingMessage("Variable not defined before use", func_name, loc_str);
    } else{
        call_node->setType(funcContext(func_name).getReturnType());
    }
}

void TypeVisitor::unaryExprAction(UnaryExprAST * unary_node) {
    // lookup possibilities for the unary opcode
    // ensure that the expression has a type, and that it matches.
    std::string opcode = std::string(unary_node->getOpCode(),1);
    // lookup

    std::shared_ptr<ExprAST> operand = unary_node->getOperand();
    operand->accept(this);
    if(!hasType(operand)){
        typingMessage("Unary operand has unkown type", "", operand->getLocStr());
        return;
    }

}

void TypeVisitor::binaryExprAction(BinaryExprAST * binary_node) {
    // lookup possibilities for the binary opcode
    // ensure that the expressions have types, and that they match
}

// Statement actions ensure that the constituent statements or expressions
// are populated with types and that any typing requirements are met.

void TypeVisitor::ifStAction(IfStatementAST * if_node){}
void TypeVisitor::forStAction(ForStatementAST * for_node){}
void TypeVisitor::whileStAction(WhileStatementAST * while_node){}
void TypeVisitor::returnStAction(ReturnStatementAST * return_node){}
void TypeVisitor::blockStAction(BlockStatementAST * block_node){}
void TypeVisitor::callStAction(CallStatementAST * call_node){}
void TypeVisitor::assignStAction(AssignStatementAST * assign_node){}
void TypeVisitor::initStAction(InitStatementAST * init_node){}

void TypeVisitor::prototypeAction(PrototypeAST * proto_node){}
void TypeVisitor::functionAction(FunctionAST * func_node){}

} // namespace typecheck
} // namespace bassoon
