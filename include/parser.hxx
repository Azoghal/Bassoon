#ifndef Bassoon_include_parser_HXX
#define Bassoon_include_parser_HXX

#include <map>
#include <memory>
#include "ast.hxx"

namespace bassoon
{

class Parser{
    static int current_token_;
    static std::map<char,int> bin_op_precedence_;
    static int getNextToken();
    static int getTokPrecedence();

    static std::unique_ptr<ExprAST> parseExpression();
    static std::unique_ptr<ExprAST> parseUnary();
    static std::unique_ptr<ExprAST> parseBinaryOpRHS(int expr_precedence, std::unique_ptr<ExprAST> lhs);
    static std::unique_ptr<ExprAST> parsePrimary();
    static std::unique_ptr<ExprAST> parseParenExpr();
    static std::unique_ptr<ExprAST> parseBoolExpr();
    static std::unique_ptr<ExprAST> parseIntExpr();
    static std::unique_ptr<ExprAST> parseDoubleExpr();
    static std::unique_ptr<ExprAST> parseIdentifierExpr();
    static std::unique_ptr<StatementAST> parseCallExpr();

    static std::unique_ptr<StatementAST> parseStatement();
    static std::unique_ptr<StatementAST> parseBlockStatement();
    static std::unique_ptr<StatementAST> parseInitStatement();
    static std::unique_ptr<StatementAST> parseAssignStatement();
    static std::unique_ptr<StatementAST> parseIfStatement();
    static std::unique_ptr<StatementAST> parseForStatement();
    static std::unique_ptr<StatementAST> parseWhileStatement();
    static std::unique_ptr<StatementAST> parseCallStatement();
    static std::unique_ptr<StatementAST> parseReturnStatement();

    static std::unique_ptr<FunctionAST> parseDefinition();

    static std::unique_ptr<PrototypeAST> parsePrototype();
    static std::unique_ptr<PrototypeAST> parseExtern();

public:
    static void setBinopPrecedence(std::map<char,int> precedents);
    //static void addBinopPrecedence(char c, int precedent);
};

} // namespace bassoon


#endif // Bassoon_include_parser_HXX