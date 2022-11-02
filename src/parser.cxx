#include "parser.hxx"
#include "lexer.hxx"
#include "log_error.hxx"
#include "tokens.hxx"

namespace bassoon
{

int Parser::current_token_ = ' ';
std::map<char,int> Parser::bin_op_precedence_ = std::map<char,int>();

int Parser::getNextToken(){
    return Parser::current_token_ = Lexer::nextTok();
}

int Parser::getTokPrecedence(){
    return Parser::bin_op_precedence_[Parser::current_token_];
}

int Parser::expectedNextToken(int expected){
    // returns 1 for expected next token
    getNextToken();
    if (current_token_ != expected)
        return 0;
    return 1;
}

 std::unique_ptr<ExprAST> Parser::parseExpression(){
    // consume a primary (that could be unaried)
    auto lhs = Parser::parseUnary();
    if (!lhs)
        return LogError<std::unique_ptr<ExprAST>>("Error in parseExpression - no LHS");

    // Try to parse a possible right hand side of the expression.
    return parseBinaryOpRHS(0, std::move(lhs));
    
 }

//  std::unique_ptr<ExprAST> Parser::parseBoolExpr(){}
//  std::unique_ptr<ExprAST> Parser::parseIntExpr(){}
//  std::unique_ptr<ExprAST> Parser::parseDoubleExpr(){}
//  std::unique_ptr<ExprAST> Parser::parseIdentifierExpr(){}
//  std::unique_ptr<ExprAST> Parser::parseIfExpr(){}
//  std::unique_ptr<ExprAST> Parser::parseForExpr(){}
//  std::unique_ptr<ExprAST> Parser::parseWhileExpr(){}
//  std::unique_ptr<ExprAST> Parser::parseVarExpr(){}
//  std::unique_ptr<ExprAST> Parser::parseParenExpr(){}
//  std::unique_ptr<ExprAST> Parser::parsePrimaryExpr(){}
//  std::unique_ptr<ExprAST> Parser::parseUnary(){}
//  std::unique_ptr<ExprAST> Parser::parseBinaryOpRHS(int expr_precedence, std::unique_ptr<ExprAST> lhs){}
std::unique_ptr<PrototypeAST> Parser::parsePrototype(){
    std::string function_name;
    SourceLoc function_loc = Lexer::getLoc();
    if(current_token_ != tok_identifier) return LogError<std::unique_ptr<PrototypeAST>>("Expected function name");
    function_name = Lexer::getIdentifier();
    if(!expectedNextToken('(')) return nullptr;
    // handle arguments
}

void Parser::setBinopPrecedence(std::map<char,int> precedences){
    Parser::bin_op_precedence_ = precedences;
}

void Parser::skipToken(){
    Parser::getNextToken();
}

std::unique_ptr<FunctionAST> Parser::parseDefinition(){
    // eat define - no need to check as it is what caused parseDefinition to be called
    getNextToken();

    // parse fnName(args of types) [gives type] 
    auto prototype = parsePrototype(); 
    if (!prototype) return nullptr;

    if(!expectedNextToken(tok_as)) return nullptr;
    if(!expectedNextToken('{')) return nullptr;
    auto body = parseExpression();
    if (!body) return nullptr;
    if(!expectedNextToken('}')) return nullptr;

    return std::make_unique<FunctionAST>(std::move(prototype), std::move(body));
}

//std::unique_ptr<PrototypeAST> Parser::parseExtern(){}

std::unique_ptr<FunctionAST> Parser::parseTopLevelExpr(){
    // make an anonymous function for the top level expression
    SourceLoc function_loc = Lexer::getLoc();
    // Parse the body and make a __main__ function with no arguments
    if (auto E = parseExpression()){
        auto Proto = std::make_unique<PrototypeAST>(function_loc, "__main__", std::vector<std::string>());
        return std::make_unique<FunctionAST> (std::move(Proto), std::move(E));
    }
    return nullptr;
}

} // namespace bassoon
