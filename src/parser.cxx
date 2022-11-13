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

std::unique_ptr<StatementAST> LogErrorS(std::string s){
    return LogError<std::unique_ptr<StatementAST>>(s);
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
            printParseAndToken("no more bin.");
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
    SourceLoc identifier_loc = Lexer::getLoc();
    std::string identifier_name = Lexer::getIdentifier();
    if(current_token_ != '('){
        getNextToken(); // consume the identifier.
        return std::make_unique<VariableExprAST>(identifier_loc, identifier_name);
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
    return std::make_unique<CallExprAST>(identifier_loc, identifier_name, std::move(args));
}

// ----------------------
// Statement Parsing
// ----------------------

std::unique_ptr<StatementAST> Parser::parseStatement(){
    printParseAndToken("Statement");
    switch(current_token_){
    case '{':
        return parseBlockStatement();
    case tok_if:
        return parseIfStatement();
    case tok_return:
        return parseReturnStatement();
    case tok_identifier:
        return parseIdentifierStatement();
    default:
        return LogErrorS("Unexpected token to start statement.");
    }
}
std::unique_ptr<StatementAST> Parser::parseBlockStatement(){
    printParseAndToken("BlockStatement");
    SourceLoc block_loc = Lexer::getLoc();
    if (current_token_ != '{')
        return LogErrorS("Expected '{' to start block of statements");
    getNextToken(); // consume '{'

    std::vector<std::unique_ptr<StatementAST>> statements;
    while(current_token_ != '}'){
        statements.push_back(parseStatement());
    }

    getNextToken(); // consume '}'
    return std::make_unique<BlockStatementAST>(block_loc, std::move(statements));
}

std::unique_ptr<StatementAST> Parser::parseIdentifierStatement(){
    printParseAndToken("id statmnt.");
    // When a statement starts with an identifier, consume this and remember the location,
    // and lookahead to get relevant parsing statement.
    SourceLoc id_loc = Lexer::getLoc();
    // current token is an identifier
    // could be call, initialiseation or assignment
    std::string id = Lexer::getIdentifier();
    getNextToken(); // consume identifier
    
    switch(current_token_){
    case '(':
        return parseCallStatement(id_loc, id);
    // case tok_of:
    //     return parseInitStatement(id_loc, id);
    case '=':
        return parseAssignStatement(id_loc, id);
    default:
        return LogErrorS("Statement contains unexpected token after identifier.");
    }
}

std::unique_ptr<StatementAST> Parser::parseCallStatement(SourceLoc id_loc, std::string id){
    printParseAndToken("callStmnt.");
    // current_token should be '(' and we need to parse the args to create a CallExprAST
    if (current_token_ != '(')
        return LogErrorS("Expect '(' to be current token at start of parseCallStatement");
    
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
                    return LogErrorS("Expected arg after ','");
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

    if(current_token_ != ')')
        return LogErrorS("Expect current token to be ')' when args parsed");
    getNextToken(); // consume ')'

    if(current_token_ != ';')
        return LogErrorS("Expected call statement to end with ';'");
    getNextToken(); // consume ';'

    printParseAndToken("callStEnd");
    auto call_expr = std::make_unique<CallExprAST>(id_loc, id, std::move(args));
    return std::make_unique<CallStatementAST>(id_loc, std::move(call_expr));
}

// std::unique_ptr<StatementAST> Parser::parseInitStatement(SourceLoc id_loc, std::string id){
//     // of type = value;
//     if (current_token_ != tok_of)
//         return LogErrorS("Expect current token to be tok_of at start of parseInit");
//     getNextToken(); // consume of

//     if(!isType(current_token_))
//         return LogErrorS("Expected a type after of in variable initialisation");
//     auto type = tokToType(current_token_);
//     getNextToken(); // consume type

//     if(current_token_ != '=')
//         return LogErrorS("Expected '=' after type in variable initialisation");
//     getNextToken(); // consume '='

//     auto value = parseExpression();
//     if(!value)
//         return LogErrorS("Error with value of assignment");
    
//     if(current_token_ != ';')
//         return LogErrorS("Expected semicolon to end variable initialisation");
//     getNextToken() // consume ';'

//     return ...;
// }

std::unique_ptr<StatementAST> Parser::parseAssignStatement(SourceLoc id_loc, std::string id){
    if(current_token_ != '=')
        return LogErrorS("Expected '=' after type in variable initialisation");
    getNextToken(); // consume '='

    auto value = parseExpression();
    if(!value)
        return LogErrorS("Error with value of assignment");
    
    if(current_token_ != ';')
        return LogErrorS("Expected semicolon to end variable initialisation");
    getNextToken(); // consume ';'

    return std::make_unique<AssignStatementAST>(id_loc, id, std::move(value));
}

std::unique_ptr<StatementAST> Parser::parseIfStatement(){
    printParseAndToken("If");
    SourceLoc if_loc = Lexer::getLoc();
    if(current_token_ != tok_if)
        return LogErrorS("Expect if statement to start with tok_if");
    getNextToken(); // consume tok_if

    std::unique_ptr<ExprAST> cond;
    std::unique_ptr<StatementAST> then, elsewise;

    cond = parseExpression();

    printParseAndToken("ifCondParsed");

    then = parseBlockStatement();
    if(!then)
        return LogErrorS("Expect statement block after condition.");
    
    if(current_token_ == tok_else){
        getNextToken(); // consume tok_else
        elsewise = parseBlockStatement();
        if (!elsewise)
            return LogErrorS("Expected statement block after else");
    }

    return std::make_unique<IfStatementAST>(if_loc, std::move(cond), std::move(then), std::move(elsewise));
}

// std::unique_ptr<StatementAST> Parser::parseForStatement();
// std::unique_ptr<StatementAST> Parser::parseWhileStatement();

std::unique_ptr<StatementAST> Parser::parseReturnStatement(){
    printParseAndToken("Return");
    SourceLoc return_loc = Lexer::getLoc();
    if (current_token_ != tok_return)
        return LogErrorS("Expect return statement to start with tok_return");
    
    getNextToken(); // consume return
    
    auto return_expr = parseExpression();
    if(!return_expr)
        return LogErrorS("Error with return expression");

    if(current_token_ != ';')
        return LogErrorS("Expected a semicolon to terminate return statement");
    getNextToken(); // consume ';'

    return std::make_unique<ReturnStatementAST>(return_loc, std::move(return_expr));
}

// std::unique_ptr<FunctionAST> Parser::parseDefinition();

// std::unique_ptr<PrototypeAST> Parser::parsePrototype();
// std::unique_ptr<PrototypeAST> Parser::parseExtern();


void Parser::mainLoop(){
    for(int i =0; i < 5; ++i){
    // while(true){
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
            auto a = parseStatement();
            if(a)
                fprintf(stderr, "Parsed Successfully\n");
        }
    //}
    }
}

} // namespace bassoon
