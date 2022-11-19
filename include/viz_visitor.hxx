#ifndef Bassoon_include_viz_visitor_HXX
#define Bassoon_include_viz_visitor_HXX

#include <map>
#include <set>
#include <fstream>
#include "ast.hxx"

namespace bassoon{
namespace viz{

class VizVisitor : public ASTVisitor{
    std::map<std::string, int> unique_namer_map_;
    // std::vector<std::string> name_stack_;
    std::ofstream output_;
    std::set<std::string> node_base_names_;
    std::string getCurrentName(std::string s);
    void advanceName(std::string s);
    std::string getAndAdvanceName(std::string s);
    void addNodeLabel(std::string name, std::string label);
public:
    VizVisitor();
    ~VizVisitor();
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

    void visualiseAST(std::shared_ptr<NodeAST> AST);
    void visualiseASTs(std::vector<std::shared_ptr<NodeAST>> ASTs);
};

} // namespace viz
} // namespace bassoon

#endif // Bassoon_include_viz_visitor_HXX