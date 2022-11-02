#include "parser.hxx"
#include "lexer.hxx"
#include "log_error.hxx"
#include "tokens.hxx"

namespace bassoon
{

std::unique_ptr<PrototypeAST> LogErrorP(const char * s){
    return LogError<std::unique_ptr<PrototypeAST>>(s);
}

std::unique_ptr<ExprAST> LogErrorE(const char * s){
    return LogError<std::unique_ptr<ExprAST>>(s);
}

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

int Parser::expectNextTokenIsType(){
    getNextToken();
    return isType(current_token_);
}

 std::unique_ptr<ExprAST> Parser::parseExpression(){
    // consume a primary (that could be unaried)
    auto lhs = Parser::parseUnary();
    if (!lhs)
        return LogError<std::unique_ptr<ExprAST>>("Error in parseExpression - no LHS");

    // Try to parse a possible right hand side of the expression.
    return parseBinaryOpRHS(0, std::move(lhs));
    
 }

std::unique_ptr<ExprAST> Parser::parseBoolExpr(){
    SourceLoc bool_loc = Lexer::getLoc();
    if (current_token_ == tok_false)
        return std::make_unique<BoolExprAST>(bool_loc,false);
    // tok_true
    return std::make_unique<BoolExprAST>(bool_loc,true);
}

//  std::unique_ptr<ExprAST> Parser::parseIntExpr(){}
//  std::unique_ptr<ExprAST> Parser::parseDoubleExpr(){}
//  std::unique_ptr<ExprAST> Parser::parseIdentifierExpr(){}
//  std::unique_ptr<ExprAST> Parser::parseIfExpr(){}
//  std::unique_ptr<ExprAST> Parser::parseForExpr(){}
//  std::unique_ptr<ExprAST> Parser::parseWhileExpr(){}
//  std::unique_ptr<ExprAST> Parser::parseVarExpr(){}
//  std::unique_ptr<ExprAST> Parser::parseParenExpr(){}
std::unique_ptr<ExprAST> Parser::parsePrimaryExpr(){
    switch(current_token_){
        default:
            return LogErrorE("Unkown token when expecting primary expression");
        case tok_false:
            return parseBoolExpr();
        case tok_true:
            return parseBoolExpr();
    }
}

std::unique_ptr<ExprAST> Parser::parseUnary(){
    SourceLoc unary_loc = Lexer::getLoc();
    if(!isascii(current_token_) || current_token_ == '('){
        //must be primary
        return parsePrimaryExpr();
    }
    int opcode = current_token_;
    if(auto operand = parseUnary())
        return std::make_unique<UnaryExprAST>(unary_loc, opcode, std::move(operand));
    return nullptr;
}

std::unique_ptr<ExprAST> Parser::parseBinaryOpRHS(int expr_precedence, std::unique_ptr<ExprAST> lhs){
    // build up tree of binop(lhs rhs) ast nodes to represent whole expression
    while (true){
        int tok_prec = getTokPrecedence(); 
        if (tok_prec < expr_precedence)
            // this is not a binop rhs
            return lhs;

        // we are in a binop
        int bin_op = current_token_;
        SourceLoc bin_loc = Lexer::getLoc();
        getNextToken(); // consume the operator
        
        auto rhs = parseUnary();
        if (!rhs)
            return nullptr;
        
        // look ahead to work out association
        int next_prec = getTokPrecedence();
        if (tok_prec < next_prec){
            // associates lhs + (rhs + ...)
            rhs = parseBinaryOpRHS(tok_prec+1, std::move(rhs));
            if (!rhs)
                return nullptr;
        }
        lhs = std::make_unique<BinaryExprAST> (bin_loc, bin_op, std::move(lhs), std::move(rhs));
    }
}

std::unique_ptr<PrototypeAST> Parser::parsePrototype(){
    std::string function_name;
    SourceLoc function_loc = Lexer::getLoc();
    if(current_token_ != tok_identifier) return LogErrorP("Expected function name");
    function_name = Lexer::getIdentifier();
    
    if(!expectedNextToken('(')) return LogErrorP("Expected '(' to start argument list");
    
    // handle arguments
    std::vector<std::pair<std::string, int>> arg_names_types;
    while(expectedNextToken(tok_identifier)){
        std::string arg_name = Lexer::getIdentifier();

        if(!expectedNextToken(tok_of)) return LogErrorP("'of' missing: [identifier] of [type] expected for each arg");
        
        getNextToken(); // move to type
        if(isType(current_token_))
           arg_names_types.push_back(std::pair<std::string,int>(arg_name,current_token_));
        else
            return LogErrorP("invalid type name");
    }
    if (current_token_ != ')') 
        return LogErrorP("Expected ')' to end argument list");

    getNextToken(); // eat ')'

    int function_type = 0; // void for now
    if (current_token_ == tok_gives){
        if(!expectNextTokenIsType())
            return LogErrorP("Expected function return type after 'gives'");
        function_type = current_token_;
    }
    // else, function type remains 0 - void

    return std::make_unique<PrototypeAST>(function_loc, function_name, std::move(arg_names_types), function_type);
}

// --------------------
// Public Methods
// --------------------

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
        auto Proto = std::make_unique<PrototypeAST>(function_loc, "__main__", std::vector<std::pair<std::string,int>>(), 0); //0 for void
        return std::make_unique<FunctionAST> (std::move(Proto), std::move(E));
    }
    return nullptr;
}

} // namespace bassoon
