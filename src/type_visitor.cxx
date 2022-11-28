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

//-----------------------------------------
// Variable and Function Context helpers
//-----------------------------------------

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

//--------------------------
// Scope helpers
//--------------------------

std::vector<std::string> TypeVisitor::getCurrentScope(){
    return scope_definitions_stack_[scope_definitions_stack_.size()-1];
}

std::vector<std::string> TypeVisitor::popCurrentScope(){
    auto temp = getCurrentScope();
    scope_definitions_stack_.pop_back();
    return temp;
}

void TypeVisitor::pushNewScope(std::vector<std::string> new_scope){
    scope_definitions_stack_.push_back(new_scope);
}

void TypeVisitor::pushToCurrentScope(std::string new_definition){
    scope_definitions_stack_[scope_definitions_stack_.size()-1].push_back(new_definition);
}

bool TypeVisitor::isInCurrentScope(std::string candidate_id){
    auto current_scope = getCurrentScope();
    for(auto id_str : current_scope){
        if (id_str == candidate_id){
            return true; 
        }
    }
    return false;
}

//--------------------
// Typing Helpers
//--------------------

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

// ---------------------
// Expr typing actions
// ---------------------

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
    char opcode = unary_node->getOpCode();
    // lookup

    std::shared_ptr<ExprAST> operand = unary_node->getOperand();
    operand->accept(this);
    if(!hasType(operand)){
        typingMessage("Unary operand has unkown type", "", operand->getLocStr());
        return;
    }

    BType operand_type = operand->getType();
    std::vector<BFType> possible_types = unary_operators[opcode];
    BType result_type = type_unknown;

    // find the typing rule that applies.
    for (BFType f_type: possible_types){
        if (f_type.getArgCount() == 1 && f_type.getArgumentTypes()[0] == operand_type){
            result_type = f_type.getReturnType();
            break;
        }
    }

    if (result_type == type_unknown){
        typingMessage("Unary operator not defined for operand type");
    }
    unary_node->setType(result_type);
}

void TypeVisitor::binaryExprAction(BinaryExprAST * binary_node) {
    // lookup possibilities for the binary opcode
    // ensure that the expressions have types, and that they match
    // lookup possibilities for the unary opcode
    // ensure that the expression has a type, and that it matches.
    char opcode = binary_node->getOpCode();
    // lookup

    std::shared_ptr<ExprAST> lhs = binary_node->getLHS();
    lhs->accept(this);
    if(!hasType(lhs)){
        typingMessage("binary operand lhs has unkown type", "", lhs->getLocStr());
        return;
    }

    std::shared_ptr<ExprAST> rhs = binary_node->getRHS();
    rhs->accept(this);
    if(!hasType(rhs)){
        typingMessage("binary operand rhs has unkown type", "", rhs->getLocStr());
        return;
    }

    BType lhs_type = lhs->getType();
    BType rhs_type = rhs->getType();
    BType result_type = type_unknown;
    std::vector<BFType> possible_types = binary_operators[opcode];

    for (BFType f_type: possible_types){
        auto f_arg_types = f_type.getArgumentTypes();
        if (f_type.getArgCount() == 2 && f_arg_types[0] == lhs_type && f_arg_types[1] == rhs_type){
            result_type = f_type.getReturnType();
            break;
        }
    }

    if (result_type == type_unknown){
        typingMessage("Binary operator not defined for operand types");
    }
    binary_node->setType(result_type);
}

// -------------------------
// Statement typing actions.
//--------------------------

// Expressions try to type themselves, but can remain without types
// Statements will check that expressions have types
// if a statement fails to type, it will raise an exception.

void TypeVisitor::ifStAction(IfStatementAST * if_node){}
void TypeVisitor::forStAction(ForStatementAST * for_node){}
void TypeVisitor::whileStAction(WhileStatementAST * while_node){}
void TypeVisitor::returnStAction(ReturnStatementAST * return_node){}
void TypeVisitor::blockStAction(BlockStatementAST * block_node){}
void TypeVisitor::callStAction(CallStatementAST * call_node){}
void TypeVisitor::assignStAction(AssignStatementAST * assign_node){}

void TypeVisitor::initStAction(InitStatementAST * init_node){
    // a of type = expr;

    // 1. a not in current scope definitions
    std::string init_id_str = init_node->getIdentifier();
    if (isInCurrentScope(init_id_str)){
        typingMessage("Identifier previously defined in this scope", init_node->getIdentifier());
        return;
    }
    BType type = init_node->getType();
    // Can safely define for this scope, so add
    addVarDefinition(init_id_str, type);

    // 2. expr of type type
    std::shared_ptr<StatementAST> assign_node = init_node->getAssignment();
    assign_node->accept(this);
    // assign node action either types well or throws
}

void TypeVisitor::prototypeAction(PrototypeAST * proto_node){}
void TypeVisitor::functionAction(FunctionAST * func_node){}

} // namespace typecheck
} // namespace bassoon
