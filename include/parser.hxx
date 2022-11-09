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
    static std::unique_ptr<ExprAST> parseCodeBlock();
    static std::unique_ptr<ExprAST> parseBoolExpr();
    static std::unique_ptr<ExprAST> parseIntExpr();
    static std::unique_ptr<ExprAST> parseDoubleExpr();
    static std::unique_ptr<ExprAST> parseIdentifierExpr();
    static std::unique_ptr<ExprAST> parseIfExpr();
    static std::unique_ptr<ExprAST> parseReturnExpr();
    static std::unique_ptr<ExprAST> parseForExpr();
    static std::unique_ptr<ExprAST> parseWhileExpr();
    static std::unique_ptr<ExprAST> parseVarExpr();
    static std::unique_ptr<ExprAST> parseParenExpr();
    static std::unique_ptr<ExprAST> parsePrimaryExpr();
    static std::unique_ptr<PrototypeAST> parsePrototype();
public:
    static void setBinopPrecedence(std::map<char,int> precedents);
    //static void addBinopPrecedence(char c, int precedent);
    static void skipToken();
    
    static std::unique_ptr<FunctionAST> parseDefinition();
    static std::unique_ptr<PrototypeAST> parseExtern();
    static std::unique_ptr<FunctionAST> parseTopLevelExpr();
};

} // namespace bassoon


#endif // Bassoon_include_parser_HXX