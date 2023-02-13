#ifndef Bassoon_include_type_visitor_HXX
#define Bassoon_include_type_visitor_HXX

#include <fstream>
#include <map>

#include "ast.hxx"

namespace bassoon{
namespace typecheck{
enum typing_phase {
    tp_lang_var = 0,
    tp_lang_fun = 1,
    tp_func_proto = 3,
    tp_user_glob = 2,
    tp_func_check = 4,
    tp_top_lvl_check =5,
};

class TypeVisitor : public ASTVisitor{
    // enum representing the phase of the typechecker
    typing_phase typecheck_phase_;
    // a map of identifier name -> stack of types (allowing ghosting)
    std::map<std::string, std::vector<BType>> identifier_stacks_;
    std::map<std::string, BFType> func_types_;
    // return_type_stack_: stack of return type of recently parsed statements
    // only return and block statements push to this stack. A block statement with
    // no return statement within will push the type_void
    std::vector<BType> return_type_stack_;
    BType popReturnType();
    void checkRetStackSize(int original_size);
    // scope_definitions_stack_: a stack of info about scopes - 
    // the identifiers that are defined in them, that need to be 
    // popped off identifier_stacks_ when the scope closes.
    std::vector<std::vector<std::string>> scope_definitions_stack_;
    std::vector<std::string> getCurrentScope();
    std::vector<std::string> popCurrentScope();
    void pushNewScope();
    void pushNewScope(std::vector<std::string> new_scope);
    void pushToCurrentScope(std::string new_definition);
    bool isInCurrentScope(std::string candidate_id);

    BType typeContext(std::string identifier); // get the type of an identifier in current scope.
    BFType funcContext(std::string func_name); 
    void addTypeContext(std::string identifier, BType type){identifier_stacks_[identifier].push_back(type);}
    void addFuncContext(std::string func_name, BFType type){func_types_[func_name] = type;}
    bool isInFuncContext(std::string candidate_f){for (auto f: func_types_){if(f.first==candidate_f){return true;}}return false;}

    void addVarDefinition(std::string identifier, BType type);
    bool varIsDefined(std::string identifier);
    bool funcIsDefined(std::string func_name);

    void printVarScopes();
    void printCurrentScopeDefinitions();

    void typecheckFuncDefs(std::shared_ptr<FuncDefs> func_defs);
    void typecheckTopLevels(std::shared_ptr<TopLevels> top_levels);
public:
    TypeVisitor();
    void boolExprAction(BoolExprAST * bool_node) override;
    void intExprAction(IntExprAST * int_node) override;
    void doubleExprAction(DoubleExprAST * double_node) override;
    void variableExprAction(VariableExprAST * variable_node) override;
    void callExprAction(CallExprAST * call_node) override;
    void unaryExprAction(UnaryExprAST * unary_node) override;
    void binaryExprAction(BinaryExprAST * binary_node) override;
    
    void ifStAction(IfStatementAST * if_node) override;
    void forStAction(ForStatementAST * for_node) override;
    void whileStAction(WhileStatementAST * while_node) override;
    void returnStAction(ReturnStatementAST * return_node) override;
    void blockStAction(BlockStatementAST * block_node) override;
    void callStAction(CallStatementAST * call_node) override;
    void assignStAction(AssignStatementAST * assign_node) override;
    void initStAction(InitStatementAST * init_node) override;

    void prototypeAction(PrototypeAST * proto_node) override;
    void functionAction(FunctionAST * func_node) override;

    void topLevelsAction(TopLevels * top_levels_node) override;
    void funcDefsAction(FuncDefs * func_defs_node) override;
    void programAction(BProgram * program_node) override;

    //void typecheckAST(NodeAST * node);
    void typecheck(std::shared_ptr<BProgram> program);
};

} // namespace typecheck
} // namespace bassoon

#endif // Bassoon_include_type_visitor_HXXA