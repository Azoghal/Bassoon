#include "type_visitor.hxx"
#include "types.hxx"
#include "inbuilt_operators.hxx"
#include "exceptions.hxx"

#include <set>

namespace bassoon
{
namespace typecheck
{

void typingMessage(std::string message){
    fprintf(stderr, "%s\n", message.c_str());
}

void typingMessage(std::string message, std::string var_or_func_name){
    fprintf(stderr, "%s: %s\n", message.c_str(),var_or_func_name.c_str());
}

void typingMessage(std::string message, std::string var_or_func_name, std::string loc_str){
    fprintf(stderr, "%s: %s at %s\n", message.c_str(), var_or_func_name.c_str(), loc_str.c_str());
}

std::string tPhaseToStr(typing_phase tp){
    switch(tp){
    case(tp_lang_var):{
        return "tp_lang_var";
    }
    case(tp_lang_fun):{
        return "tp_lang_fun";
    }
    case(tp_func_proto):{
        return "tp_func_proto";
    }
    case(tp_user_glob):{
        return "tp_user_glob";
    }
    case(tp_func_check):{
        return "tp_func_check";
    }
    case(tp_top_lvl_check):{
        return "tp_top_lvl_check";
    }
    default:{
        return "not a typing phase";
    }
    }
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

// void TypeVisitor::typecheckAST(NodeAST * node){
//     node->accept(this);
//     printVarScopes();
// }

void TypeVisitor::typecheck(std::shared_ptr<BProgram> program){
    // std::shared_ptr<FuncDefs> func_defs = program->getFuncDefs();
    // std::shared_ptr<TopLevels> top_levels = program->getTopLevels();
    // // Typecheck functions and populate function context
    // typecheckFuncDefs(func_defs);
    // // Typecheck statements
    // typecheckTopLevels(top_levels);
    program->accept(this);
}

void TypeVisitor::typecheckFuncDefs(std::shared_ptr<FuncDefs> func_defs){

}

void TypeVisitor::typecheckTopLevels(std::shared_ptr<TopLevels> top_levels){
    
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

void TypeVisitor::addVarDefinition(std::string identifier, BType type){
    pushToCurrentScope(identifier);
    addTypeContext(identifier, type);
    printVarScopes();
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
    auto old_scope_vars = getCurrentScope();
    scope_definitions_stack_.pop_back();
    // remove any definitions made in the scope popped.
    std::vector<std::string> now_empties;
    for (auto old_scope_var : old_scope_vars){
        identifier_stacks_[old_scope_var].pop_back();
        if (identifier_stacks_[old_scope_var].size()==0){
            now_empties.push_back(old_scope_var);
        }
    }
    // erase any variable that no longer has a definition
    for(auto empty_var : now_empties){
        auto it = identifier_stacks_.find(empty_var);
        if (it != identifier_stacks_.end()){
            identifier_stacks_.erase(it);
        }
    }
    return old_scope_vars;
}

void TypeVisitor::pushNewScope(){
    scope_definitions_stack_.push_back(std::vector<std::string>());
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

void TypeVisitor::printCurrentScopeDefinitions(){
    std::vector<std::string> current_scope = getCurrentScope();
    int columns = current_scope.size();
    int max_height = 1;
    fprintf(stderr,"\n");
    // 5 characters for column, 1 character gap
    for(auto var:current_scope){
        // stack big enough to print one.
        fprintf(stderr,"%5s ",typeToStr(typeContext(var)).c_str());
    }
    fprintf(stderr,"\n");
    for(auto var:current_scope){
        fprintf(stderr,"%5s ",var.c_str());
    }
    fprintf(stderr,"\nCurrent Scope Definitions\n");
}


void TypeVisitor::printVarScopes(){
    // print all the stacks of identifiers
    int columns = identifier_stacks_.size();
    int max_height = 0;
    for(auto var_stack: identifier_stacks_){
        if (var_stack.second.size() > max_height){
            max_height = var_stack.second.size();
        }
    }
    fprintf(stderr,"\n");
    // 5 characters for column, 1 character gap
    for(int i = max_height-1; i >= 0; --i){
        for(auto var_stack:identifier_stacks_){
            if(var_stack.second.size()>i){
                // stack big enough to print one.
                fprintf(stderr,"%5s ",typeToStr(var_stack.second[i]).c_str());
            }else{
                fprintf(stderr,"      "); // to make columns allign
            }
        }
        fprintf(stderr,"\n");
    }
    for(auto var_stack:identifier_stacks_){
        fprintf(stderr,"%5s ",var_stack.first.c_str());
    }
    fprintf(stderr,"\nOverall Stacks\n");
    printCurrentScopeDefinitions();
}

//-------------------
//  Return Type Helpers
//-------------------

BType TypeVisitor::popReturnType(){
    if(return_type_stack_.size() == 0){
        typingMessage("Ran out of return types");
        throw BError();
    }
    else{
        BType top_type = return_type_stack_[return_type_stack_.size()-1];
        return_type_stack_.pop_back();
        return top_type;
    }
}

void TypeVisitor::checkRetStackSize(int original_size){
    if(original_size != return_type_stack_.size()){
        typingMessage("Return type stack size differs from original expected.", std::to_string(original_size), std::to_string(return_type_stack_.size()));
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
        typingMessage("Function not defined before use", func_name, loc_str);
        return; //throw BError();
    }
    
    BFType func_type = funcContext(func_name);

    // Try to type all the args
    call_node->resetArgIndex();
    while(call_node->anotherArg()){
        call_node->argAcceptOne(this); 
    }

    std::vector<BType> expected_arg_types = func_type.getArgumentTypes();
    // Check their types match
    call_node->resetArgIndex();
    for(int arg_i=0; call_node->anotherArg(); ++arg_i){
        auto arg_expr = call_node->getOneArg();
        if(arg_expr.getType() != expected_arg_types[arg_i]){
            std::string arg_type_str = 
                "Exp: " + typeToStr(expected_arg_types[arg_i])
                + "Actual: " + typeToStr(arg_expr.getType())
                + arg_expr.getLocStr();
            typingMessage("Arg doesn't match", func_name, arg_type_str);
            return; // throw?
        }
    }
    call_node->setType(func_type.getReturnType());
}

void TypeVisitor::unaryExprAction(UnaryExprAST * unary_node) {
    // lookup possibilities for the unary opcode
    // ensure that the expression has a type, and that it matches.
    char opcode = unary_node->getOpCode();
    // lookup

    unary_node->operandAccept(this);
    auto operand = unary_node->getOperand();
    if(!hasType(operand)){
        typingMessage("Unary operand has unkown type", "", operand.getLocStr());
        return;
    }

    BType operand_type = operand.getType();
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

    binary_node->lhsAccept(this);
    auto lhs = binary_node->getLHS();
    if(!hasType(lhs)){
        typingMessage("binary operand lhs has unkown type", "", lhs.getLocStr());
        return;
    }

    binary_node->rhsAccept(this);
    auto rhs = binary_node->getRHS();
    if(!hasType(rhs)){
        typingMessage("binary operand rhs has unkown type", "", rhs.getLocStr());
        return;
    }

    BType lhs_type = lhs.getType();
    BType rhs_type = rhs.getType();
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

void TypeVisitor::ifStAction(IfStatementAST * if_node){
    // Condition must be bool
    // Statements must type check and must agree if they have return types.
    int original_ret_size = return_type_stack_.size();

    if_node->condAccept(this);
    auto cond_node = if_node->getCond();
    if (!hasType(cond_node)){
        typingMessage("If statement condition expression failed to type","",cond_node.getLocStr());
        return; // throw
    }
    if(cond_node.getType() != type_bool){
        typingMessage("If statement condition not a bool expression","",cond_node.getLocStr());
        throw BError();
    }

    if_node->thenAccept(this);
    auto then_node = if_node->getThen();
    BType then_ret_type = popReturnType();

    if_node->elseAccept(this);
    auto else_node = if_node->getElse();
    BType else_ret_type = popReturnType();

    checkRetStackSize(original_ret_size);

    if(then_ret_type != type_void && else_ret_type != type_void ){
        if(then_ret_type == else_ret_type){
            // both have a return and they match
            return_type_stack_.push_back(else_ret_type);
            return;
        }else{
            typingMessage("If statement then and else blocks have different return types.",then_node.getLocStr(),if_node->getLocStr());
            throw BError();
        }
    }
    else if(then_ret_type == type_void){
        return_type_stack_.push_back(else_ret_type);
        return;
    }
    else if(else_ret_type == type_void){
        return_type_stack_.push_back(then_ret_type);
        return;
    }
    typingMessage("ERROR - if return type checking code broken");
}

void TypeVisitor::forStAction(ForStatementAST * for_node){
    int original_ret_size = return_type_stack_.size();

    //push a new scope for the loop (will contain the induction variables)
    pushNewScope();

    // start and step statements should type well
    for_node->startAccept(this);
    for_node->stepAccept(this);

    // end condition a well typed bool expression
    // check after start as will likely have defined induction variable.
    for_node->endAccept(this);
    auto end_node = for_node->getEnd();
    if(!hasType(end_node)){
        typingMessage("For statement condition not well typed");
        throw BError();
    }
    if(end_node.getType() != type_bool){
        typingMessage("For statement condition not a bool");
        throw BError();
    }

    // Ret stack should have two more
    checkRetStackSize(original_ret_size+2);
    // Ignore the start and step statement return types
    popReturnType();
    popReturnType();

    for_node->bodyAccept(this);
    checkRetStackSize(original_ret_size+1);
    // Leave the body's return type as the for statement's return type.

    // pop the induction variable scope
    popCurrentScope();
}

void TypeVisitor::whileStAction(WhileStatementAST * while_node){
    // end condition a well typed bool expression
    int original_ret_size = return_type_stack_.size();
    while_node->condAccept(this);
    auto end_node = while_node->getCond();
    if(!hasType(end_node)){
        typingMessage("While statement condition not well typed");
        throw BError();
    }
    if(end_node.getType() != type_bool){
        typingMessage("While statement condition not a bool");
        throw BError();
    }
    
    // Type Check the body
    while_node->bodyAccept(this);
    checkRetStackSize(original_ret_size+1);
    // Leave the body's return type as the for statement's return type.
}

void TypeVisitor::returnStAction(ReturnStatementAST * return_node){
    return_node->returnExprAccept(this);
    auto expr_node = return_node->getReturnExpr();
    if (!hasType(expr_node)){
        typingMessage("Return expression failed to type","",expr_node.getLocStr());
        return; // throw
    }
    typingMessage("Adding return type to stack from return at ", return_node->getLocStr());
    return_type_stack_.push_back(expr_node.getType());
}

void TypeVisitor::blockStAction(BlockStatementAST * block_node){
    // A block statement can contain a return statement
    // Other constructs have block statements and therefore can contain return statements
    // Block statement typechecks if all its sub statements typecheck
    // and it has agreeing return types.
    // Visit each statement in order - how to capture if its a return or a block statement?
    // Bool Handled Recent Return
    // BType Most Recent Return  - stack? and assert always empty
    
    // Can ignore the lower portion of the stack as it is in a larger scope
    // Must leave the stack one larger - with the agreed return type of this block
    // (if there is agreement)
    pushNewScope();
    int original_ret_size = return_type_stack_.size();
    int isc; // inner statement count
    block_node->resetStatementIndex();
    for(isc = 0; block_node->anotherStatement(); ++isc){
        // check that the statements type well
        block_node->statementAcceptOne(this);
    }
    printVarScopes();
    // Now need to check that return types match

    typingMessage("statement block inner statements: ", std::to_string(isc), block_node->getLocStr());
    int number_pushed = return_type_stack_.size() - original_ret_size;
    typingMessage("number of pushed ret types", std::to_string(number_pushed));

    // pop all the contained returned types and add them to the set
    std::set<BType> contained_return_types;
    for(int st_i = number_pushed; st_i>0;--st_i){
        contained_return_types.insert(popReturnType());
    }

    const bool has_void = contained_return_types.find(type_void) != contained_return_types.end();
    int ret_type_count = contained_return_types.size();
    if (ret_type_count > 2 || (!has_void && ret_type_count > 1)){
        std::string disagreed = "";
        for (BType bt : contained_return_types){
            disagreed.append(typeToStr(bt));
            disagreed.append(" ");
        }
        typingMessage("Disagreement in return type", disagreed);
        throw BError();
    }

    BType return_type = type_void;
    // find the other type if it exists
    for(BType bt : contained_return_types){
        if (bt!=type_void){
            return_type = bt;
            break;
        }
    }
    //assert size of stack is same as when entered this block
    checkRetStackSize(original_ret_size);
    return_type_stack_.push_back(return_type);
    popCurrentScope();
}


void TypeVisitor::callStAction(CallStatementAST * call_node){
    std::string function_name = call_node->getCall().getName();
    if (!isInFuncContext(function_name)){
        typingMessage("Function not defined at use",function_name,call_node->getLocStr());
        throw BError();
    }
    BFType func_type = funcContext(function_name);

    // Arg type checking done by the expression
    call_node->callAccept(this);
    if(!hasType(call_node->getCall())){
        typingMessage("Call statement - call expression doesn't type");
        throw BError();
    }
    // call statements have their return value thrown away
    return_type_stack_.push_back(type_void);
}

void TypeVisitor::assignStAction(AssignStatementAST * assign_node){
    // var = expr
    // 1. a in scope definitions
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
    return_type_stack_.push_back(type_void);
}

void TypeVisitor::initStAction(InitStatementAST * init_node){
    // a of type = expr;
    // 1. a not in current scope definitions
    std::string init_id_str = init_node->getIdentifier();
    if (isInCurrentScope(init_id_str)){
        typingMessage("Identifier previously defined in this scope", init_node->getIdentifier());
        return; //throw
    }
    BType type = init_node->getType();
    // Can safely define for this scope, so add
    addVarDefinition(init_id_str, type);

    // 2. expr of type type
    //std::shared_ptr<AssignStatementAST> assign_node = init_node->getAssignment();
    // assign node action either types well or throws
    init_node->assignmentAccept(this);
    popReturnType(); // pop the void from assignment
    return_type_stack_.push_back(type_void); // add a void for the initialisation
}

void TypeVisitor::prototypeAction(PrototypeAST * proto_node){
    switch(typecheck_phase_){
    case(tp_func_proto):{
        typingMessage("adding prototype to context");
        // adding the prototypes, so check not already defined.
        std::string f_name = proto_node->getName();
        if(isInFuncContext(f_name)){
            typingMessage("Function already defined",f_name, proto_node->getLocStr());
            throw BError();
        }
        BFType f_type = proto_node->getType();
        addFuncContext(f_name, f_type);
        break;
    }
    case(tp_func_check):{
        typingMessage("adding proto var definitions for body typecheck");
        // checking the function body so populate var scopes with arguments
        for (auto [var_id, var_type] : proto_node->getArgs()){
            addVarDefinition(var_id,var_type);
        }
        break;
    }
    default:{
        typingMessage("func proto typechecked in unexpected phase: ", tPhaseToStr(typecheck_phase_));
    }
    }
}

void TypeVisitor::functionAction(FunctionAST * func_node){
    switch(typecheck_phase_){
    case(tp_func_proto):{
        // populate func and var context with proto accept
        typingMessage("Accepting a function in proto phase");
        func_node->protoAccept(this);
        break;
    }
    case(tp_func_check):{
        typingMessage("Accepting function in func typecheck phase");
        int original_ret_size = return_type_stack_.size();
        BType return_type = func_node->getType().getReturnType();
        // Push new scope for argument definitions
        pushNewScope();

        // add the function arguments to the scope
        func_node->protoAccept(this);

        // validate that body is well typed
        func_node->bodyAccept(this);
        popCurrentScope();

        // validate that return type matches prototype
        BType body_return_type = popReturnType();
        checkRetStackSize(original_ret_size);
        if(body_return_type != return_type){
            typingMessage("Function body does not have same return type as prototype", func_node->getProto().getName(), func_node->getLocStr());
            throw BError();
        }
        break;
    }
    default:{
        typingMessage("func typechecked in unexpected phase: ", tPhaseToStr(typecheck_phase_));
    }
    }
}

void TypeVisitor::topLevelsAction(TopLevels * top_levels_node){
    switch(typecheck_phase_){
    case tp_user_glob:{
        typingMessage("Add top level globals to context phase, not yet in language");
        break;
    }
    case tp_top_lvl_check:{
        // typecheck the top level statements
        typingMessage("accepting all top level statements in top level typecheck phase");
        top_levels_node->statementsAllAccept(this);
        break;
    }
    default:{
        typingMessage("Unexpected call to topLevelsAction in typecheck phase", tPhaseToStr(typecheck_phase_));
    }
    } 
}

void TypeVisitor::funcDefsAction(FuncDefs * func_defs_node){
    switch(typecheck_phase_){
    case tp_func_proto:{
        // Find protos and add them to context
        typingMessage("accepting all functions in proto phase");
        func_defs_node->functionsAllAccept(this);
        break;
    }
    case tp_func_check:{
        // typecheck the bodies against the protos
        typingMessage("accepting all functions in function check phase");
        func_defs_node->functionsAllAccept(this);
        break;
    }
    default:{
        typingMessage("Unexpected call to funcDefsAction in typecheck phase", tPhaseToStr(typecheck_phase_));
    }
    }
}

void TypeVisitor::programAction(BProgram * program_node){
    // 1  - add language globals/constants to variable context
    // 2  - add language functions to function context
    // 3  - add all the function def prototypes to the function context
    // 4  - currently no globals, but would need to add them to variable context here
    // 5 - typecheck all the function definitions
    // 6 - typecheck all the top level statements (even if some functions didn't typecheck)
    
    typingMessage("TYPECHECKING");

    // Phase 1 - language variable context
    typecheck_phase_ = tp_lang_var;
    typingMessage("Phase 1",tPhaseToStr(typecheck_phase_));
    // go over all the language variables

    // Phase 2 - language function context
    typecheck_phase_ = tp_lang_fun;
    typingMessage("Phase 2",tPhaseToStr(typecheck_phase_));
    // go over all the language inbuilt functions

    // Phase 3 - function prototypes
    typecheck_phase_ = tp_func_proto;
    typingMessage("Phase 3",tPhaseToStr(typecheck_phase_));
    program_node->funcDefsAccept(this);

    // Phase 4 - user globals (currently not in language)
    typecheck_phase_ = tp_user_glob;
    typingMessage("Phase 4",tPhaseToStr(typecheck_phase_));
    program_node->topLevelsAccept(this);
    

    // Phase 5 - function typecheck
    typecheck_phase_ = tp_func_check;
    typingMessage("Phase 5",tPhaseToStr(typecheck_phase_));
    program_node->funcDefsAccept(this);

    // Phase 6 - top level statement typecheck
    typecheck_phase_ = tp_top_lvl_check;
    typingMessage("Phase 6",tPhaseToStr(typecheck_phase_));
    program_node->topLevelsAccept(this);
}

} // namespace typecheck
} // namespace bassoon
