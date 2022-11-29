#ifndef Bassoon_include_type_visitor_HXX
#define Bassoon_include_type_visitor_HXX

#include <fstream>
#include <map>

#include "ast.hxx"

namespace bassoon{
namespace typecheck{

class TypeVisitor : public ASTVisitor{
    // a map of identifier name -> stack of types (allowing ghosting)
    std::map<std::string, std::vector<BType>> identifier_stacks_;
    std::map<std::string, BFType> func_types_;
    // scope_definitions_stack_: a stack of info about scopes - 
    // the identifiers that are defined in them, that need to be 
    // popped off identifier_stacks_ when the scope closes.
    std::vector<std::vector<std::string>> scope_definitions_stack_;
    std::vector<std::string> getCurrentScope();
    std::vector<std::string> popCurrentScope();
    void pushNewScope(std::vector<std::string> new_scope);
    void pushToCurrentScope(std::string new_definition);
    bool isInCurrentScope(std::string candidate_id);

    BType typeContext(std::string identifier); // get the type of an identifier in current scope.
    BFType funcContext(std::string func_name); // need a new return type?? would probs help in all sorts of places
    void addTypeContext(std::string identifier, BType type){identifier_stacks_[identifier].push_back(type);}
    void addFuncContext(std::string func_name, BFType type){func_types_[func_name] = type;}
    bool isInFuncContext(std::string candidate_f){for (auto f: func_types_){if(f.first==candidate_f){return true;}}return false;}

    void addVarDefinition(std::string identifier, BType type){pushToCurrentScope(identifier); addTypeContext(identifier, type);}
    bool varIsDefined(std::string identifier);
    bool funcIsDefined(std::string func_name);

    void printVarScopes();
public:
    TypeVisitor();
    ~TypeVisitor();
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

    void typeCheckAST(NodeAST * node);
};

} // namespace typecheck
} // namespace bassoon

#endif // Bassoon_include_type_visitor_HXX