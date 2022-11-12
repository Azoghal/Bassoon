#include "parser.hxx"
#include "lexer.hxx"
#include "log_error.hxx"
#include "tokens.hxx"

namespace bassoon
{

int Parser::current_token_ = ' ';
std::map<char,int> Parser::bin_op_precedence_ = std::map<char,int>({{'-', 10}, {'+', 20}, {'/', 30}, {'*', 40}});
std::function<int()> Parser::bassoon_nextTok_ = Lexer::nextTok;
int Parser::verbosity_ = 0;


void Parser::setSource(std::function<int()> source){
    // Not strictly needed but protects bassoon_getchar as private attr.
    bassoon_nextTok_ = source;
}

int Parser::getNextToken(){
    return current_token_ = bassoon_nextTok_();
    //return Parser::current_token_ = Lexer::nextTok();
}


void Parser::printParseAndToken(std::string parseFunction){
    if (verbosity_ > 0){
        fprintf(stderr, "Parsing %15s, " , parseFunction.c_str());
        fprintf(stderr, "%3i ", current_token_);
        fprintf(stderr, " %c  ", current_token_);
        fprintf(stderr, "%s\n",  tokToStr(current_token_).c_str());
    }
    
}

std::unique_ptr<PrototypeAST> LogErrorP(std::string s){
    return LogError<std::unique_ptr<PrototypeAST>>(s);
}

std::unique_ptr<ExprAST> LogErrorE(std::string s){
    return LogError<std::unique_ptr<ExprAST>>(s);
}

std::unique_ptr<FunctionAST> LogErrorF(std::string s){
    return LogError<std::unique_ptr<FunctionAST>>(s);
}

int Parser::getTokPrecedence(){
    // some token, not an operator
    if(!isascii(current_token_))
        return -1;

    int tok_prec = Parser::bin_op_precedence_[Parser::current_token_];
    // set to -1 if not an entry
    if (tok_prec <= 0)
        tok_prec = -1; 
    return tok_prec;
}

std::unique_ptr<ExprAST> Parser::parseExpression(){
    printParseAndToken("Expression");    // consume a primary (that could be unaried)
    auto lhs = Parser::parseUnary();
    if (!lhs)
        return LogError<std::unique_ptr<ExprAST>>("Error in parseExpression - no LHS");

    // Try to parse a possible right hand side of the expression.
    printParseAndToken("Expr, nextBinop");
    return parseBinaryOpRHS(0, std::move(lhs));
}

std::unique_ptr<ExprAST> Parser::parseUnary(){
    printParseAndToken("Unary");
    SourceLoc unary_loc = Lexer::getLoc();
    // !isascii(current_token_) 
    // means guaranteed to be a keyword and hence some other expression.
    if (!isascii(current_token_) || current_token_ == '('){
        printParseAndToken("unary->primary");
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
    printParseAndToken("Primary");
    switch (current_token_){
        default:
            printParseAndToken("parsePrim.Error");
            return LogErrorE("Unknown token when expecting an expression");
        case tok_identifier:
            return parseIdentifierExpr();
        case tok_true:
            return parseBoolExpr();
        case tok_false:
            return parseBoolExpr();
        case tok_number_int:
            return parseIntExpr();
        case tok_number_double:
            return parseDoubleExpr();
        case '(':
            return parseParenExpr();
    }
}

std::unique_ptr<ExprAST> Parser::parseBinaryOpRHS(int expr_precedence, std::unique_ptr<ExprAST> lhs){
    printParseAndToken("BinaryOpRHS");
    // lhs op rhs
    // lhs op rhs1 op rhs2
    // build up tree of binop(lhs rhs) ast nodes to represent whole expression
    while (true){
        // Operators have positive precedence. Non operators have negative precedence
        int tok_prec = getTokPrecedence(); 
        printParseAndToken("binopCmp");
        if (tok_prec < expr_precedence){
            // this is not a binop rhs
            return lhs;
        }
        // we are in a binop
        int bin_op = current_token_;
        SourceLoc bin_loc = Lexer::getLoc();
        getNextToken(); // consume the operator
        printParseAndToken("bin op");
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
    printParseAndToken("Paren");
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
    printParseAndToken("Bool");
    SourceLoc bool_loc = Lexer::getLoc();
    if (current_token_ == tok_false){
        getNextToken();
        return std::make_unique<BoolExprAST>(bool_loc, false);
    }
    else if (current_token_ == tok_true){
        getNextToken(); // consume the bool
        return std::make_unique<BoolExprAST>(bool_loc, true);
    }
    return nullptr;
}

std::unique_ptr<ExprAST> Parser::parseIntExpr(){
    printParseAndToken("Int");
    SourceLoc int_loc = Lexer::getLoc();
    if (current_token_ == tok_number_int){
        int int_val = Lexer::getInt();
        getNextToken(); // consume the number
        return std::make_unique<IntExprAST>(int_loc, int_val);
    }
    return nullptr;
}

std::unique_ptr<ExprAST> Parser::parseDoubleExpr(){
    printParseAndToken("Double");
    SourceLoc double_loc = Lexer::getLoc();
    if (current_token_ == tok_number_double){
        double double_val = Lexer::getDouble();
        getNextToken(); // consume the number
        return std::make_unique<DoubleExprAST>(double_loc, double_val);
    }
    return nullptr;
}

std::unique_ptr<ExprAST> Parser::parseIdentifierExpr(){
    printParseAndToken("Identifier");
    SourceLoc literal_loc = Lexer::getLoc();
    std::string identifier_name = Lexer::getIdentifier();
    if(current_token_ != '('){
        getNextToken(); // consume the identifier.
        return std::make_unique<VariableExprAST>(literal_loc, identifier_name);
    }
       
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

// std::unique_ptr<StatementAST> Parser::parseStatement();

// std::unique_ptr<StatementAST> Parser::parseBlockStatement();

// std::unique_ptr<StatementAST> Parser::parseInitStatement();
// std::unique_ptr<StatementAST> Parser::parseAssignStatement();
// std::unique_ptr<StatementAST> Parser::parseIfStatement();
// std::unique_ptr<StatementAST> Parser::parseForStatement();
// std::unique_ptr<StatementAST> Parser::parseWhileStatement();
// std::unique_ptr<StatementAST> Parser::parseCallStatement();
// std::unique_ptr<StatementAST> Parser::parseReturnStatement();

// std::unique_ptr<FunctionAST> Parser::parseDefinition();

// std::unique_ptr<PrototypeAST> Parser::parsePrototype();
// std::unique_ptr<PrototypeAST> Parser::parseExtern();

void Parser::mainLoop(){
    //while(true){
    printParseAndToken("mainLoop");
    switch(current_token_){
    case tok_eof:
        return;
    case 32: // ' '
        getNextToken();
    // case tok_define;
    //     parseDefinition();
    // case tok_extern:
    //     parseExtern();
    default:
        auto a = parseExpression();
        if(a)
            fprintf(stderr, "Parsed Successfully");
        //parseStatement();
    }
    //}
}

} // namespace bassoon
