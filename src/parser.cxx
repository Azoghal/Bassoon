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

std::unique_ptr<ExprAST> Parser::parseCodeBlock(){
    // { expression 1; expression2; ...}
    if (current_token_ != '{')
        return LogErrorE("Error in parseCodeBlock, code block should start with '{'\n");

    getNextToken(); // move to first 

    auto expr = parseExpression(); // replace with loop for multiple statements
    if(!expectedNextToken(';'))
        return LogErrorE("Error in parseCodeBlock, expected ';' to finish statement\n");  
    
    if(!expectedNextToken('}'))
        return LogErrorE("Erorr in parseCodeBlock, code block should end with }\n");
    return expr;
}

std::unique_ptr<ExprAST> Parser::parseExpression(){
    // consume a primary (that could be unaried)
    fprintf(stderr, "starting parse unary\n");
    auto lhs = Parser::parseUnary();
    if (!lhs)
        fprintf(stderr, "not lhs\n");
        return LogError<std::unique_ptr<ExprAST>>("Error in parseExpression - no LHS");


    fprintf(stderr, "doing parse binary\n");
    // Try to parse a possible right hand side of the expression.
    return parseBinaryOpRHS(0, std::move(lhs));
    
}

std::unique_ptr<ExprAST> Parser::parseBoolExpr(){
    SourceLoc bool_loc = Lexer::getLoc();
    if (current_token_ == tok_false){
        fprintf(stderr, "parsed a bool false");
        return std::make_unique<BoolExprAST>(bool_loc,false);
    }
    if (current_token_ == tok_true){
        fprintf(stderr, "parsed a bool true");
        return std::make_unique<BoolExprAST>(bool_loc,true);
    }
    return nullptr;
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
    fprintf(stderr, "parsing unary : token = %c %s\n", current_token_, tokToStr(current_token_).c_str());
    if(!isascii(current_token_) || current_token_ == '('){
        //must be primary
        fprintf(stderr, "its a primary\n");
        return parsePrimaryExpr();
    }

    int opcode = current_token_;
    if(auto operand = parseUnary()){
        fprintf(stderr, "parsed a unary");
        return std::make_unique<UnaryExprAST>(unary_loc, opcode, std::move(operand));
    }
    return nullptr;
}

std::unique_ptr<ExprAST> Parser::parseBinaryOpRHS(int expr_precedence, std::unique_ptr<ExprAST> lhs){
    // lhs op rhs
    // lhs op rhs1 op rhs2
    // build up tree of binop(lhs rhs) ast nodes to represent whole expression
    while (true){
        int tok_prec = getTokPrecedence(); 
        if (tok_prec < expr_precedence){
            // this is not a binop rhs
            fprintf(stderr, "Parsed a binary op rhs");
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

std::unique_ptr<PrototypeAST> Parser::parsePrototype(){
    // funName(arg1 of type1, ...) gives returnType
    // funName() 
    // Leaves current_token = tok_as
    std::string function_name;
    SourceLoc function_loc = Lexer::getLoc();
    if(current_token_ != tok_identifier){
        std::string error_msg = "Expected function name, got " + tokToStr(current_token_);
        return LogErrorP(error_msg.c_str());
    } 
    function_name = Lexer::getIdentifier();
    
    if(!expectedNextToken('(')) return LogErrorP("Expected '(' to start argument list");
    
    // handle arguments
    // (), (arg of type), (arg1 of type1, arg2 of type2)
    std::vector<std::pair<std::string, int>> arg_names_types;
    bool has_args = false;
    std::vector<int> arg_starters = {tok_identifier, ','};
    while(expectNextTokenIsOneOf(arg_starters)){
        if (current_token_ == ','){
            if (!has_args) 
                return LogErrorP("Cannot start arg list with ','");
            //consume ','
            if(!expectedNextToken(tok_identifier))
                return LogErrorP("Expected arg name after ','");
        }

        // current_token  is tok_identifier for arg name
        std::string arg_name = Lexer::getIdentifier();
        has_args = true;

        if(!expectedNextToken(tok_of)) return LogErrorP("'of' missing: [identifier] of [type] expected for each arg");
        
        int arg_type = getNextToken(); // move to type
        if(isType(current_token_)){
            fprintf(stderr, "Arg pair: %s of %s\n", arg_name.c_str(), tokToStr(arg_type).c_str());
            arg_names_types.push_back(std::pair<std::string,int>(arg_name, arg_type));
        } else {
            return LogErrorP("Invalid type name");
        }
    }
    if (current_token_ != ')') 
        return LogErrorP("Expected ')' to end argument list");

    getNextToken(); // eat ')'

    int function_type = type_void; // void for now
    if (current_token_ == tok_gives){
        if(!expectNextTokenIsType())
            return LogErrorP("Expected function return type after 'gives'");
        function_type = tokToType(current_token_);
        getNextToken(); // move to tok_as
    }
    // 
    fprintf(stderr, "Parsed a prototype, return type %s\n", typeToStr(function_type).c_str());
    return std::make_unique<PrototypeAST>(function_loc, function_name, std::move(arg_names_types), function_type);
}

// --------------------
// Public Methods
// --------------------

std::unique_ptr<FunctionAST> Parser::parseDefinition(){
    // define funName(arg1 of type 1, arg2 of type2 ...) gives returnType as {body}
    // define funName() as {body} -- void function with no arguments 
    
    // consume define - no need to check as it is what caused parseDefinition to be called
    getNextToken();

    // parse fnName(args of types) [gives type] 
    auto prototype = parsePrototype(); 
    if (!prototype) return LogErrorF("definition failed, invalid prototype");

    fprintf(stderr, "made it here\n");

    if(current_token_ != tok_as) return LogErrorF("expected 'as' after function prototype");
    // replace with a 'parseCodeBlock' function?

    getNextToken(); // move to code block

    fprintf(stderr, "made it here\n");

    auto body = parseCodeBlock();
    if (!body) return LogErrorF("definition failed, problem parsing function body");
    fprintf(stderr, "made it here\n");

    fprintf(stderr, "Parsed a definition\n");
    return std::make_unique<FunctionAST>(std::move(prototype), std::move(body));
}

//std::unique_ptr<PrototypeAST> Parser::parseExtern(){}

std::unique_ptr<FunctionAST> Parser::parseTopLevelExpr(){
    // make an anonymous function for the top level expression
    SourceLoc function_loc = Lexer::getLoc();
    // Parse the body and make a __main__ function with no arguments
    if (auto E = parseExpression()){
        auto Proto = std::make_unique<PrototypeAST>(function_loc, "__main__", std::vector<std::pair<std::string,int>>(), 0); //0 for void
        fprintf(stderr, "Parsed a top level expression\n");
        return std::make_unique<FunctionAST> (std::move(Proto), std::move(E));
    }
    return nullptr;
}

void Parser::setBinopPrecedence(std::map<char,int> precedences){
    Parser::bin_op_precedence_ = precedences;
}

void Parser::skipToken(){
    Parser::getNextToken();
}

} // namespace bassoon
