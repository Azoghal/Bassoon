#include "parser.hxx"
#include "lexer.hxx"
#include "log_error.hxx"
#include "tokens.hxx"

namespace bassoon
{

std::unique_ptr<PrototypeAST> LogErrorP(std::string s){
    return LogError<std::unique_ptr<PrototypeAST>>(s);
}

std::unique_ptr<ExprAST> LogErrorE(std::string s){
    return LogError<std::unique_ptr<ExprAST>>(s);
}

std::unique_ptr<FunctionAST> LogErrorF(std::string s){
    return LogError<std::unique_ptr<FunctionAST>>(s);
}

int Parser::current_token_ = ' ';
std::map<char,int> Parser::bin_op_precedence_ = std::map<char,int>();

int Parser::getNextToken(){
    return Parser::current_token_ = Lexer::nextTok();
}

int Parser::getTokPrecedence(){
    return Parser::bin_op_precedence_[Parser::current_token_];
}

std::unique_ptr<ExprAST> Parser::parseExpression(){
    // consume a primary (that could be unaried)
    auto lhs = Parser::parseUnary();
    if (!lhs)
        return LogError<std::unique_ptr<ExprAST>>("Error in parseExpression - no LHS");

    // Try to parse a possible right hand side of the expression.
    return parseBinaryOpRHS(0, std::move(lhs));
}

std::unique_ptr<ExprAST> Parser::parseUnary(){
    SourceLoc unary_loc = Lexer::getLoc();
    // !isascii(current_token_) 
    // means guaranteed to be a keyword and hence some other expression.
    if (!isascii(current_token_) || current_token_ == '('){
        return parsePrimary();
    }
    // otherwise it must be an operator
    int op_code = current_token_;
    getNextToken();
    if (auto operand = parseUnary())
        return std::make_unique<UnaryExprAST>(unary_loc, op_code, std::move(operand));
    
    return nullptr;
}

std::unique_ptr<ExprAST> Parser::parsePrimary(){
    switch (current_token_){
        default:
            return LogErrorE("Unknown token when expecting an expression");
        case tok_identifier:
            return parseIdentifierExpr();
        case tok_bool:
            return parseBoolExpr();
        case tok_int:
            return parseIntExpr();
        case tok_double:
            return parseDoubleExpr();
        case '(':
            return parseParenExpr();
    }
}

std::unique_ptr<ExprAST> Parser::parseBinaryOpRHS(int expr_precedence, std::unique_ptr<ExprAST> lhs){
    // lhs op rhs
    // lhs op rhs1 op rhs2
    // build up tree of binop(lhs rhs) ast nodes to represent whole expression
    while (true){
        int tok_prec = getTokPrecedence(); 
        if (tok_prec < expr_precedence){
            // this is not a binop rhs
            return lhs;
        }
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

std::unique_ptr<ExprAST> Parser::parseParenExpr(){
    getNextToken(); // consume '('
    auto expression = parseExpression();
    if (!expression)
        return nullptr;

    if (current_token_ != ')'){
        return LogErrorE("Expected ')' to end paren expression");
    }
    getNextToken(); // consume ')'
    return expression; 
}

std::unique_ptr<ExprAST> Parser::parseBoolExpr(){
    SourceLoc bool_loc = Lexer::getLoc();
    if (current_token_ == tok_false)
        return std::make_unique<BoolExprAST>(bool_loc, false);
    else if (current_token_ == tok_true)
        return std::make_unique<BoolExprAST>(bool_loc, true);
    return nullptr;
}

std::unique_ptr<ExprAST> Parser::parseIntExpr(){
    SourceLoc int_loc = Lexer::getLoc();
    if (current_token_ == tok_number_int)
        return std::make_unique<IntExprAST>(int_loc, Lexer::getInt());
    return nullptr;
}

std::unique_ptr<ExprAST> Parser::parseDoubleExpr(){
    SourceLoc double_loc = Lexer::getLoc();
    if (current_token_ == tok_number_double)
        return std::make_unique<DoubleExprAST>(double_loc, Lexer::getDouble());
    return nullptr;
}

std::unique_ptr<ExprAST> Parser::parseIdentifierExpr(){
    SourceLoc literal_loc = Lexer::getLoc();
    std::string identifier_name = Lexer::getIdentifier();
    if(current_token_ != '(')
        return std::make_unique<VariableExprAST>(literal_loc, identifier_name);
    
    getNextToken(); // consume '('
    std::vector<std::unique_ptr<ExprAST>> args;
    bool expecting_another_arg = false;
    if (current_token_ != ')'){
        while(true){
            if (auto arg = parseExpression()){
                args.push_back(std::move(arg));
                expecting_another_arg = false;
            }
            else{
                if (expecting_another_arg)
                    return LogErrorE("Expected arg after ','");
                return nullptr;
            }
            
            if (current_token_ == ')')
                break;
            
            if (current_token_ == ','){
                // consume ',' between args to advance to next
                expecting_another_arg = true;
                getNextToken(); 
            }
        }
    }
    getNextToken(); // consume ')'
    return std::make_unique<CallExprAST>(literal_loc, identifier_name, std::move(args));
}

// static std::unique_ptr<StatementAST> parseStatement();
// static std::unique_ptr<StatementAST> parseBlockStatement();
// static std::unique_ptr<StatementAST> parseInitStatement();
// static std::unique_ptr<StatementAST> parseAssignStatement();
// static std::unique_ptr<StatementAST> parseIfStatement();
// static std::unique_ptr<StatementAST> parseForStatement();
// static std::unique_ptr<StatementAST> parseWhileStatement();
// static std::unique_ptr<StatementAST> parseCallStatement();
// static std::unique_ptr<StatementAST> parseReturnStatement();

// static std::unique_ptr<FunctionAST> parseDefinition();

// static std::unique_ptr<PrototypeAST> parsePrototype();
// static std::unique_ptr<PrototypeAST> parseExtern();

} // namespace bassoon
