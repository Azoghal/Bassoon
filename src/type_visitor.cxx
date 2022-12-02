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

//----------------------------
// Constructor
//----------------------------

TypeVisitor::TypeVisitor(){
    // push an initially empty top level scope
    scope_definitions_stack_.push_back(std::vector<std::string>());
}

//------------------------------
// Public Driving Function
//------------------------------

void TypeVisitor::typecheckAST(std::shared_ptr<NodeAST> node){
    fprintf(stderr, "getting top level to accept\n");
    node->accept(this);
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
    fprintf(stderr,"how many scopes? %lu", scope_definitions_stack_.size());
    return scope_definitions_stack_[scope_definitions_stack_.size()-1];
}

std::vector<std::string> TypeVisitor::popCurrentScope(){
    auto old_scope_vars = getCurrentScope();
    scope_definitions_stack_.pop_back();
    // remove any definitions made in the scope popped.
    for (auto old_scope_var : old_scope_vars){
        identifier_stacks_[old_scope_var].pop_back();
    }
    return old_scope_vars;
}

void TypeVisitor::pushNewScope(std::vector<std::string> new_scope){
    scope_definitions_stack_.push_back(new_scope);
}

void TypeVisitor::pushToCurrentScope(std::string new_definition){
    scope_definitions_stack_[scope_definitions_stack_.size()-1].push_back(new_definition);
}

bool TypeVisitor::isInCurrentScope(std::string candidate_id){
    fprintf(stderr,"getting current scope in isInCurrentScope\n");
    auto current_scope = getCurrentScope();
    fprintf(stderr,"got it\n");
    for(auto id_str : current_scope){
        if (id_str == candidate_id){
            return true; 
        }
    }
    return false;
}


void TypeVisitor::printVarScopes(){
    // print all the stacks of identifires
    int collumns = identifier_stacks_.size();
    int max_height = 0;
    for(auto var_stack: identifier_stacks_){
        if (var_stack.second.size() > max_height){
            max_height = var_stack.second.size();
        }
    }
    // 5 characters for column, 1 character gap
    for(int i = max_height-1; i >= 0; --i){
        for(auto var_stack:identifier_stacks_){
            if(var_stack.second.size()>i){
                // stack big enough to print one.
                fprintf(stderr,"%5s ",typeToStr(var_stack.second[i]).c_str());
            }
        }
        fprintf(stderr,"\n");
    }
    for(auto var_stack:identifier_stacks_){
        fprintf(stderr,"%5s ",var_stack.first.c_str());
    }
}

//--------------------
// Typing Helpers
//--------------------

bool hasType(ExprAST node){
    if (node.getType() == type_unknown){
        return false;
    }
    return true;
}

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

void TypeVisitor::assignStAction(AssignStatementAST * assign_node){
    // var = expr
    // 1. a in scope definitions
    fprintf(stderr,"in assign action\n");
    std::string assigned_var = assign_node->getIdentifier();
    if(!varIsDefined(assigned_var)){
        typingMessage("Assigned variable not defined", assigned_var, assign_node->getLocStr());
        return; // throw
    }
    // var is defined
    BType defined_type = typeContext(assigned_var);

    // 2. expr types 
    // std::shared_ptr<ExprAST> value_expr = assign_node->getValue();
    // value_expr->accept(this);
    assign_node->valueAccept(this);
    auto value_expr = assign_node->getValue();
    if(!hasType(value_expr)){
        typingMessage("Assignment value not well typed", value_expr.getLocStr());
        return; // throw
    }
    BType val_expr_type = value_expr.getType();

    // 3. expr type matches uppermost var definition;
    if (val_expr_type != defined_type){
        typingMessage("Variable and expression type do not match", typeToStr(defined_type)+typeToStr(val_expr_type), assign_node->getLocStr());
        return; // throw
    }
}

void TypeVisitor::initStAction(InitStatementAST * init_node){
    // a of type = expr;
    fprintf(stderr,"init action\n");
    // 1. a not in current scope definitions
    std::string init_id_str = init_node->getIdentifier();
    fprintf(stderr,"init identifier is: %s\n", init_id_str.c_str());
    if (isInCurrentScope(init_id_str)){
        typingMessage("Identifier previously defined in this scope", init_node->getIdentifier());
        return; //throw
    }
    fprintf(stderr,"getting init type\n");
    BType type = init_node->getType();
    // Can safely define for this scope, so add
    addVarDefinition(init_id_str, type);

    // 2. expr of type type
    //std::shared_ptr<AssignStatementAST> assign_node = init_node->getAssignment();
    fprintf(stderr,"calling assign accept within init\n");
    // assign node action either types well or throws
    //init_node->getAssignment().accept(this);
    init_node->getAssignment().getCol();
    init_node->assignmentAccept(this);
}

void TypeVisitor::prototypeAction(PrototypeAST * proto_node){}
void TypeVisitor::functionAction(FunctionAST * func_node){}

} // namespace typecheck
} // namespace bassoon
