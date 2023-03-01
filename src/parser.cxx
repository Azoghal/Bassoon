#include "parser.hxx"
#include "lexer.hxx"
#include "log_error.hxx"
#include "tokens.hxx"
#include "exceptions.hxx"

namespace bassoon
{

int Parser::current_token_ = ' ';
std::map<char,int> Parser::bin_op_precedence_ = std::map<char,int>({{'<', 5}, {'>',6}, {'-', 10}, {'+', 20}, {'/', 30}, {'*', 40}});
std::function<int()> Parser::bassoon_nextTok_ = Lexer::nextTok;
int Parser::verbosity_ = 0;


void Parser::setSource(std::function<int()> source){
    // Not strictly needed but protects bassoon_getchar as private attr.
    bassoon_nextTok_ = source;
}

int Parser::getNextToken(){
    printParseAndToken("--");
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
    getNextToken(); // move onto '(' or next token if not a call
    if(current_token_ != '('){
        //getNextToken(); // consume the identifier.
        return std::make_unique<VariableExprAST>(identifier_loc, identifier_name);
    }
       
    getNextToken(); // consume '('
    printParseAndToken("IdIsFun");
    std::vector<std::unique_ptr<ExprAST>> args;
    bool expecting_another_arg = false;
    if (current_token_ != ')'){
        printParseAndToken("AnArg");
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
    case tok_for:
        return parseForStatement();
    case tok_while:
        return parseWhileStatement();
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
        printParseAndToken("start in block");
        statements.push_back(parseStatement());
        printParseAndToken("sttmnt in block");
    }

    printParseAndToken("} after block");
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
    case tok_of:
        return parseInitStatement(id_loc, id);
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

std::unique_ptr<StatementAST> Parser::parseInitStatement(SourceLoc id_loc, std::string id){
    // of type = value;
    printParseAndToken("Init");

    if (current_token_ != tok_of)
        return LogErrorS("Expect current token to be tok_of at start of parseInit");
    getNextToken(); // consume of

    if(!tokIsType(current_token_))
        return LogErrorS("Expected a type after of in variable initialisation");
    BType type = tokToType(current_token_);
    getNextToken(); // consume type

    SourceLoc assign_loc = Lexer::getLoc();
    if(current_token_ != '=')
        return LogErrorS("Expected '=' after type in variable initialisation");
    getNextToken(); // consume '='

    printParseAndToken("InitPreExpr");
    auto value_expr = parseExpression();
    if(!value_expr)
        return LogErrorS("Error with value_expr of assignment");
    printParseAndToken("InitEnd");
    if(current_token_ != ';')
        return LogErrorS("Expected semicolon to end variable initialisation");
    getNextToken(); // consume ';'

    auto assignment = std::make_unique<AssignStatementAST>(assign_loc, id, std::move(value_expr));
    return std::make_unique<InitStatementAST>(id_loc, id, type, std::move(assignment));
}

std::unique_ptr<StatementAST> Parser::parseAssignStatement(SourceLoc id_loc, std::string id){
    printParseAndToken("assign");
    
    if(current_token_ != '=')
        return LogErrorS("Expected '=' after type in variable assignment");
    getNextToken(); // consume '='

    auto value = parseExpression();
    if(!value)
        return LogErrorS("Error with value of assignment");
    
    printParseAndToken("assign semi");
    if(current_token_ != ';')
        return LogErrorS("Expected semicolon to end variable assignment");
    getNextToken(); // consume ';'
    printParseAndToken("as. after se");

    return std::make_unique<AssignStatementAST>(id_loc, id, std::move(value));
}

std::unique_ptr<StatementAST> Parser::parseIfStatement(){
    printParseAndToken("If");
    SourceLoc if_loc = Lexer::getLoc();
    if(current_token_ != tok_if)
        return LogErrorS("Expect if statement to start with tok_if");
    getNextToken(); // consume tok_if

    if(current_token_ != '(')
        return LogErrorS("Expect if statement condition within parentheses.");
    getNextToken(); // consume '('

    std::unique_ptr<ExprAST> cond;
    std::unique_ptr<StatementAST> then, elsewise;

    cond = parseExpression();
    printParseAndToken("ifCondParsed");

    if(current_token_ != ')')
        return LogErrorS("Expected ')' to end condition expression.");
    getNextToken(); // consume ')'


    then = parseBlockStatement();
    if(!then)
        return LogErrorS("Expect statement block after condition.");
    
    bool has_else = false;
    if(current_token_ == tok_else){
        has_else=true;
        getNextToken(); // consume tok_else
        elsewise = parseBlockStatement();
        if (!elsewise)
            return LogErrorS("Expected statement block after else");
    }

    return std::make_unique<IfStatementAST>(if_loc, std::move(cond), std::move(then), std::move(elsewise), has_else);
}

std::unique_ptr<StatementAST> Parser::parseForStatement(){
    // for (setup identifier statement; comp exp; step identifier statement) block_statement
    printParseAndToken("for");
    SourceLoc for_loc = Lexer::getLoc();

    if (current_token_ != tok_for)
        return LogErrorS("Expected for statement to start with 'for'.");
    getNextToken(); // consume 'for'

    if(current_token_ != '(')
        return LogErrorS("Expected '(' to start for loop induction statements.");
    getNextToken(); // consume '('

    printParseAndToken("forSetup");
    auto setup_statement = parseIdentifierStatement(); // call(), var of type = val or var = val;
    if(!setup_statement)
        return LogErrorS("Error with for loop setup statement");
    
    printParseAndToken("forCond");
    auto condition_expr = parseExpression();
    if(!condition_expr)
        return LogErrorS("Error with for loop condition expression");

    // parseExpression doesn't consume ';'
    if (current_token_ != ';')
        return LogErrorS("Expected ';' to separate condition expression from step statement.");
    getNextToken(); // consume ';'
    
    printParseAndToken("forStep");
    auto step_statement = parseIdentifierStatement();
    if(!step_statement)
        return LogErrorS("Error with for loop step statement");

    if(current_token_ != ')')
        return LogErrorS("Expected ')' to end for loop induction statements.");
    getNextToken(); // consume ')'

    printParseAndToken("forBody");
    auto body = parseBlockStatement();
    if(!body)
        return LogErrorS("Error with for loop body");

    return std::make_unique<ForStatementAST>(for_loc, std::move(setup_statement), std::move(condition_expr),std::move(step_statement), std::move(body));
}

std::unique_ptr<StatementAST> Parser::parseWhileStatement(){
    // while (expr) statement block
    printParseAndToken("While");
    SourceLoc while_loc = Lexer::getLoc();
    
    if(current_token_ != tok_while)
        return LogErrorS("Expect while statement to start with 'while'");
    getNextToken(); // consume while

    if(current_token_ != '(')
        return LogErrorS("Expected '(' to start for loop induction statements.");
    getNextToken(); // consume '('

    printParseAndToken("whileCond");
    auto condition_expr = parseExpression();
    if(!condition_expr)
        return LogErrorS("Error with while loop condition expression");
    
    if(current_token_ != ')')
        return LogErrorS("Expected ')' to end while loop condition expression");
    getNextToken(); // consume ')'

    auto body = parseBlockStatement();
    if(!body)
        return LogErrorS("Error with while loop body");
    
    return std::make_unique<WhileStatementAST>(while_loc, std::move(condition_expr), std::move(body));
}

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

std::unique_ptr<FunctionAST> Parser::parseDefinition(){
    // define foo(var of type...) [gives type] as statement block   
    printParseAndToken("dfntion.");
    SourceLoc def_loc = Lexer::getLoc();

    if(current_token_ != tok_define)
        return LogErrorF("Expect function definition to start with define");
    getNextToken(); // consume define

    auto proto = parsePrototype();
    if(!proto)
        return LogErrorF("Error with function prototype");
    
    if(current_token_!=tok_as)
        return LogErrorF("Expect current token to be tok_as after prototype parse");
    getNextToken(); // consume tok_as

    auto body = parseBlockStatement();
    if(!body)
        return LogErrorF("Error with function body");
    
    return std::make_unique<FunctionAST>(def_loc, std::move(proto), std::move(body));
}

std::unique_ptr<PrototypeAST> Parser::parsePrototype(){
    // foo([var of type]*) [gives type]
    printParseAndToken("prototype");
    SourceLoc proto_loc = Lexer::getLoc();

    std::string function_name;
    BType return_type;

    std::vector<BType> func_arg_types;

    std::string arg_name;
    BType arg_type;
    std::vector<std::pair<std::string,BType>> args_and_types;

    if(current_token_ != tok_identifier)
        return LogErrorP("Expected function prototype to start with identifier");
    function_name = Lexer::getIdentifier();
    getNextToken(); // consume function name

    if(current_token_ != '(')
        return LogErrorP("Expected '(' to start function arguments");
    getNextToken(); // consume '('

    bool expecting_arg = false;
    while (current_token_ != ')'){
        // var of type [,]
        printParseAndToken("protoAList");

        if(current_token_ != tok_identifier)
            return LogErrorP("Expect arguments in form [identifier of type]");
        arg_name = Lexer::getIdentifier();
        getNextToken(); // consume arg name

        if(current_token_ != tok_of)
            return LogErrorP("Expect [of type] after identifier");
        getNextToken(); // consume of

        if(!tokIsType(current_token_))
            return LogErrorP("Expected type for argument");
        arg_type = tokToType(current_token_);
        getNextToken(); // consume type

        // Have name and type so add to arg list
        args_and_types.push_back(std::pair<std::string,BType>(arg_name, arg_type));
        func_arg_types.push_back(arg_type);
        expecting_arg = false;

        if(current_token_ == ','){
            expecting_arg = true;
            getNextToken(); // consume , and expect another argument
        } 

    }
    if (expecting_arg)
        return LogErrorP("Stray ',' at end of argument list");
    getNextToken(); // consume ')' (guarunteed by loop breaking)

    printParseAndToken("prtoListDone");
    if(current_token_ == tok_gives){
        getNextToken(); // consume gives
        if(!tokIsType(current_token_))
            return LogErrorP("Expect return type after gives");
        return_type = tokToType(current_token_);
        getNextToken(); // consume return type;
    }
    else if(current_token_ == tok_as){
        return_type = type_void;
    }
    else{
        return LogErrorP("Expected [gives type] or as ... after args list");
    }

    BFType func_type(func_arg_types,return_type);
    return std::make_unique<PrototypeAST>(proto_loc, function_name, args_and_types, func_type);   
}
// std::unique_ptr<PrototypeAST> Parser::parseExtern();


std::unique_ptr<BProgram> Parser::parseLoop(){
    std::vector<std::unique_ptr<StatementAST>> top_level_statements;
    std::vector<std::unique_ptr<FunctionAST>> function_definitions;
    while(true){
        printParseAndToken("mainLoop");
        switch(current_token_){
            case 32: { // ' '
                getNextToken();
                break;
            };
            case tok_define: {
                printParseAndToken("define");
                auto def = parseDefinition();
                if(!def){
                    fprintf(stderr,"Error parsing definition\n");
                    throw BError();
                }
                function_definitions.push_back(std::move(def));
                if(verbosity_)
                    fprintf(stderr, "Parsed Definitionn Successfully\n");
                break;
            };
            case tok_eof: {
                printParseAndToken("EOF");
                std::unique_ptr<TopLevels> top_levels = std::make_unique<TopLevels>(std::move(top_level_statements));
                std::unique_ptr<FuncDefs> func_defs = std::make_unique<FuncDefs>(std::move(function_definitions));
                return std::make_unique<BProgram>(std::move(top_levels), std::move(func_defs));
            };
            default: {
                printParseAndToken("default");
                auto statement = parseStatement();
                if(!statement){
                    fprintf(stderr,"Error parsing statement\n");
                    throw BError();
                }
                top_level_statements.push_back(std::move(statement));
                if(verbosity_)
                    fprintf(stderr, "Parsed Top Level Statement Successfully\n");
            };
        }
    }
    // *
    // std::unique_ptr<TopLevels> top_levels = std::make_unique<TopLevels>(std::move(top_level_statements));
    // std::unique_ptr<FuncDefs> func_defs = std::make_unique<FuncDefs>(std::move(function_definitions));
    // return std::make_unique<BProgram>(std::move(top_levels), std::move(func_defs));
}

} // namespace bassoon
