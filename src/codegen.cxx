#include "ast.hxx"
#include "codegen.hxx"
#include "exceptions.hxx"

#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

namespace bassoon
{
namespace codegen
{

CodeGenerator::CodeGenerator(){
    context_ = std::make_unique<llvm::LLVMContext>();
    module_ = std::make_unique<llvm::Module>("doubleReed", *context_);
    builder_ = std::make_unique<llvm::IRBuilder<>>(*context_);
}

void CodeGenerator::printIR(){
    module_->print(llvm::errs(), nullptr);
}

void CodeGenerator::definePutChar(){
    std::vector<llvm::Type *> arg_types;
    arg_types.push_back(llvm::Type::getInt32Ty(*context_));
    llvm::FunctionType * func_type = llvm::FunctionType::get(llvm::Type::getInt32Ty(*context_), arg_types, false); 
    llvm::Function * func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "putchar", module_.get());
}

void CodeGenerator::generate(std::shared_ptr<BProgram> program){
    program->accept(this);
}

void CodeGenerator::optimize(){
    // Create the analysis managers
    llvm::LoopAnalysisManager loop_analysis;
    llvm::FunctionAnalysisManager function_analysis;
    llvm::CGSCCAnalysisManager CGSCC_analysis;
    llvm::ModuleAnalysisManager module_analysis;

    // Create a new pass manager builder
    llvm::PassBuilder pass_builder;

    // Register the analyses with the managers
    pass_builder.registerModuleAnalyses(module_analysis);
    pass_builder.registerCGSCCAnalyses(CGSCC_analysis);
    pass_builder.registerFunctionAnalyses(function_analysis);
    pass_builder.registerLoopAnalyses(loop_analysis);
    pass_builder.crossRegisterProxies(loop_analysis, function_analysis, CGSCC_analysis, module_analysis);

    // Create a pass manager
    // Corresponds to typical -O2 optimization pipeline in clang.
    llvm::ModulePassManager optimisation_pass_manager = pass_builder.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O2);
    
    // Do the optimisations
    spdlog::info("BEFORE OPTIMIZATION ---------------------------");
    this->printIR();
    optimisation_pass_manager.run(*module_, module_analysis); 
    spdlog::info("AFTER OPTIMIZATION ----------------------------");
    this->printIR();

}

void CodeGenerator::setTarget(){
    std::string target_triple = llvm::sys::getDefaultTargetTriple();
    if(target_triple != "x86_64-unknown-linux-gnu"){
        spdlog::info("Initialising all targets");
        llvm::InitializeAllTargetInfos();
        llvm::InitializeAllTargets();
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllAsmParsers();
        llvm::InitializeAllAsmPrinters();
    }else{
        llvm::InitializeNativeTarget();
        llvm::InitializeNativeTargetAsmParser();
        llvm::InitializeNativeTargetAsmPrinter();
    }

    std::string target_lookup_error;
    auto target = llvm::TargetRegistry::lookupTarget(target_triple, target_lookup_error);
    if(!target){
        llvm::errs()<<target_lookup_error;
        spdlog::error("{0}", target_lookup_error);
        throw BError();
    }
    
    std::string cpu = "generic";
    std::string features = "";
    llvm::TargetOptions options;
    auto relocation_model = llvm::Optional<llvm::Reloc::Model>();
    target_machine_ = target->createTargetMachine(target_triple, cpu, features, options, relocation_model);
    module_->setDataLayout(target_machine_->createDataLayout());
    module_->setTargetTriple(target_triple);
}

void CodeGenerator::compile(){
    std::string object_filename = "output.o";
    std::error_code EC;
    llvm::raw_fd_ostream destination (object_filename, EC, llvm::sys::fs::OF_None);
    if(EC){
        llvm::errs() << "Can't open file " << EC.message();
        spdlog::error("Can't open file {0}", EC.message());
        throw BError();
    }

    llvm::legacy::PassManager code_gen_pass_manager;
    auto file_type = llvm::CGFT_ObjectFile; // code gen file type

    if(target_machine_->addPassesToEmitFile(code_gen_pass_manager, destination, nullptr, file_type)){
        llvm::errs() << "target machine can't emit a file of this type";
        return;
    }

    code_gen_pass_manager.run(*module_);
    destination.flush();
}


// ----------------------
// Helpers
// ----------------------

llvm::AllocaInst * CodeGenerator::createEntryBlockAlloca(llvm::Function *function, llvm::Argument * arg){
    // Make an IR builder pointing to first instruction of entry block.
    llvm::IRBuilder<> temp_builder (&function->getEntryBlock(), function->getEntryBlock().begin());
    return temp_builder.CreateAlloca(arg->getType(),0,arg->getName());
}

llvm::Type * CodeGenerator::convertBType(BType btype){
    // Handle function types?
    switch(btype){
    case(type_void):{
        return llvm::Type::getVoidTy(*context_);
    }
    case(type_bool):{
        return llvm::Type::getInt1Ty(*context_);
    }
    case(type_int):{
        return llvm::Type::getInt32Ty(*context_);
    }
    case(type_double):{
        return llvm::Type::getDoubleTy(*context_);
    }
    default:{
        spdlog::error("{0} is not a convertible type", typeToStr(btype));
        throw BError();
    }
    }
}


BType CodeGenerator::convertLlvmType(llvm::Type * type){
    if(type==convertBType(type_bool)){
        return type_bool;
    }
    else if(type==convertBType(type_int)){
        return type_int;
    }else if(type==convertBType(type_double)){
        return type_double;
    }
    spdlog::error("Unknown llvm type to convert to BType");
    throw BError();
}

llvm::Value * CodeGenerator::popLlvmValue(){
    if (!(llvm_value_stack_.size()>0)){
        spdlog::error("llvm_value_stack is empty but a pop was attempted");
        throw BError();
    }
    llvm::Value * val = llvm_value_stack_[llvm_value_stack_.size()-1];
    llvm_value_stack_.pop_back();
    spdlog::debug("POPPED, size: {0:d}",llvm_value_stack_.size());
    return val;
}

void CodeGenerator::pushLlvmValue(llvm::Value * val){
    if(!val){
        spdlog::error("Value to be pushed is null");
        throw BError();
    }
    llvm_value_stack_.push_back(val);
}

llvm::Function * CodeGenerator::popLlvmProto(){
    if (!(llvm_proto_stack_.size()>0)){
        spdlog::error("llvm_proto_stack is empty but a pop was attempted");
        throw BError();
    }
    llvm::Function * f = llvm_proto_stack_[llvm_proto_stack_.size()-1];
    llvm_proto_stack_.pop_back();
    return f;
}

void CodeGenerator::pushLlvmProto(llvm::Function * proto){
    if(!proto){
        spdlog::error("Proto to be pushed is null");
        throw BError();
    }
    llvm_proto_stack_.push_back(proto);
}

// -----------------------
//  Casting
// -----------------------

llvm::Value * CodeGenerator::createCast(llvm::Value * val, BType dest_type){
    if (dest_type == type_double){
        return tryIntToDoubleCast(val);
    }
    else if(dest_type == type_int){
        return tryDoubleToIntCast(val);
    }
    spdlog::error("not a castable type");
    throw BError();
}

llvm::Value * CodeGenerator::createIntToDoubleCast(llvm::Value * int_val){
    spdlog::debug("Casting int to double");
    return builder_->CreateSIToFP(int_val, convertBType(type_double),"int_to_double_cast");
}
llvm::Value * CodeGenerator::createDoubleToIntCast(llvm::Value * double_val){
    spdlog::debug("Casting double to int");
    return builder_->CreateFPToSI(double_val, convertBType(type_int), "double_to_int_cast");
}

llvm::Value * CodeGenerator::tryIntToDoubleCast(llvm::Value * maybe_int_val){
    if(maybe_int_val->getType()->isIntegerTy()){
        spdlog::debug("int cast required");
        return this->createIntToDoubleCast(maybe_int_val);
    }
    else if(maybe_int_val->getType()->isFloatingPointTy()){
        spdlog::debug("No int cast required");
        return maybe_int_val;
    }
    spdlog::error("Unexpected type given to tryIntToDoubleCast. Float? {0} Int? {1}", maybe_int_val->getType()->isFloatingPointTy(), maybe_int_val->getType()->isIntegerTy());
    throw BError();
}
llvm::Value * CodeGenerator::tryDoubleToIntCast(llvm::Value * maybe_double_val){
    if(maybe_double_val->getType()->isFloatingPointTy()){
        spdlog::debug("double cast required");
        return this->createDoubleToIntCast(maybe_double_val);
    }
    else if(maybe_double_val->getType()->isIntegerTy()){
        spdlog::debug("No double cast required");
        return maybe_double_val;
    }
    spdlog::error("Unexpected type given to tryDoubleToIntCast");
    throw BError();
}

//------------------------
// Builder Helpers
//------------------------

llvm::Value * CodeGenerator::createAdd(BType res_type, llvm::Value * lhs_val, llvm::Value * rhs_val){
    switch(res_type){
    case(type_int):{
        return builder_->CreateAdd(lhs_val,rhs_val,"int_bin_add_temp");
    }
    case(type_double):{
        return builder_->CreateFAdd(lhs_val,rhs_val,"double_bin_add_temp");
    }
    default:{
        spdlog::error("Type is not a valid result of sum operation");
        throw BError();
    }
    }
}

llvm::Value * CodeGenerator::createSub(BType res_type, llvm::Value * lhs_val, llvm::Value * rhs_val){
    switch(res_type){
    case(type_int):{
        return builder_->CreateSub(lhs_val,rhs_val,"int_bin_sub_temp");
    }
    case(type_double):{
        return builder_->CreateFSub(lhs_val,rhs_val,"double_bin_sub_temp");
    }
    default:{
        spdlog::error("Type is not a valid result of sub operation");
        throw BError();
    }
    }
}

llvm::Value * CodeGenerator::createMul(BType res_type, llvm::Value * lhs_val, llvm::Value * rhs_val){
    switch(res_type){
    case(type_int):{
        return builder_->CreateMul(lhs_val,rhs_val,"int_bin_mul_temp");
    }
    case(type_double):{
        return builder_->CreateFMul(lhs_val,rhs_val,"double_bin_mul_temp");
    }
    default:{
        spdlog::error("Type is not a valid result of mul operation");
        throw BError();
    }
    }
}

llvm::Value * CodeGenerator::createDiv(BType res_type, llvm::Value * lhs_val, llvm::Value * rhs_val){
    switch(res_type){
    case(type_int):{
        return builder_->CreateSDiv(lhs_val,rhs_val,"int_bin_div_temp");
    }
    case(type_double):{
        return builder_->CreateFDiv(lhs_val,rhs_val,"double_bin_div_temp");
    }
    default:{
        spdlog::error("Type {0} is not a valid result of div operation", typeToStr(res_type));
        throw BError();
    }
    }
}

llvm::Value * CodeGenerator::createLessThan(BType lhs_type, BType rhs_type, llvm::Value * lhs_val, llvm::Value * rhs_val){
    if(lhs_type == rhs_type && lhs_type == type_int){
        return builder_->CreateICmpSLT(lhs_val, rhs_val, "int_cmp_lt");
    }
    else if(lhs_type == rhs_type && lhs_type == type_double){
        return builder_->CreateFCmpOLT(lhs_val,rhs_val,"double_cmp_lt");
    }
    else{
        return builder_->CreateFCmpOLT(tryIntToDoubleCast(lhs_val),tryIntToDoubleCast(rhs_val),"double_cmp_lt");
    }
}

llvm::Value * CodeGenerator::createGreaterThan(BType lhs_type, BType rhs_type, llvm::Value * lhs_val, llvm::Value * rhs_val){
    spdlog::debug("Making >, types: l: {0} r: {1}", typeToStr(lhs_type), typeToStr(rhs_type));
    if(lhs_type == rhs_type && lhs_type == type_int){
        return builder_->CreateICmpSGT(lhs_val, rhs_val, "int_cmp_gt");
    }
    else if(lhs_type == rhs_type && lhs_type == type_double){
        return builder_->CreateFCmpOGT(lhs_val,rhs_val,"double_cmp_gt");
    }
    else{
        return builder_->CreateFCmpOGT(tryIntToDoubleCast(lhs_val),tryIntToDoubleCast(rhs_val),"double_cmp_gt");
    }
}

llvm::Value * CodeGenerator::createAnd(BType res_type, llvm::Value * lhs_val, llvm::Value * rhs_val){
    return builder_->CreateAnd(lhs_val, rhs_val);
}

llvm::Value * CodeGenerator::createOr(BType res_type, llvm::Value * lhs_val, llvm::Value * rhs_val){
    return builder_->CreateOr(lhs_val, rhs_val);
}

//--------------------
// Expression Actions
//--------------------

void CodeGenerator::boolExprAction(BoolExprAST * bool_node){
    llvm::Value * bool_const = llvm::ConstantInt::get(llvm::Type::getInt1Ty(*context_),bool_node->getValue(),false);
    pushLlvmValue(bool_const);
}

void CodeGenerator::intExprAction(IntExprAST * int_node){
    llvm::Value * int_const = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_),int_node->getValue(),true);
    pushLlvmValue(int_const);
}

void CodeGenerator::doubleExprAction(DoubleExprAST * double_node){
    llvm::Value * double_const = llvm::ConstantFP::get(*context_,llvm::APFloat(double_node->getValue()));
    pushLlvmValue(double_const);
}

void CodeGenerator::variableExprAction(VariableExprAST * variable_node){
    std::string name = variable_node->getName();
    std::string loc_str = variable_node->getLocStr();
    spdlog::debug("gening variable {0} expression at {1}",name, variable_node->getLocStr());
    llvm::Type * llvm_type = convertBType(variable_node->getType());
    llvm::Value *var_val = named_values_[variable_node->getName()];
    if(!var_val){
        spdlog::error("Variable name unknown {0} at {1}",name, loc_str);
        throw BError();
    }
    // Seg Fault HERE when for loop step uses variable
    llvm::Value * load_val = builder_->CreateLoad(llvm_type, var_val, name);
    pushLlvmValue(load_val);
}

void CodeGenerator::callExprAction(CallExprAST * call_node){
    llvm::Function * callee_func = module_->getFunction(call_node->getName());
    if(!callee_func){
        spdlog::error("Unknown function called");
        throw BError();
    }

    // codegen the args and pop them off into args vec
    std::vector<BType> callee_arg_types = call_node->getCalleeType().getArgumentTypes();
    std::vector<llvm::Value *> args_vec;
    call_node->resetArgIndex();
    for(int i = 0; call_node->anotherArg(); i++){
        call_node->argAcceptOne(this);
        llvm::Value * arg_val = popLlvmValue();
        BType arg_type = convertLlvmType(arg_val->getType());
        BType expected_type = callee_arg_types[i];
        if(arg_type != expected_type){
            spdlog::debug("arg type not a match {0} expected: {1}", typeToStr(arg_type), typeToStr(expected_type));
            if(isCastable(arg_type,expected_type)){
                spdlog::debug("creating cast");
                arg_val = createCast(arg_val,expected_type);
            }
            else{
                throw BError();
            }
        }
        args_vec.push_back(arg_val);
    }

    if(callee_func->arg_size() != args_vec.size()){
        spdlog::error("mismatch arg size");
        throw BError();
    }

    llvm::Value * ret_val;
    if (callee_func->getReturnType()->isVoidTy()){
        ret_val = builder_->CreateCall(callee_func,args_vec);
    }
    else{
        ret_val = builder_->CreateCall(callee_func,args_vec,"calltmp");
    }
    pushLlvmValue(ret_val);
}

void CodeGenerator::unaryExprAction(UnaryExprAST * unary_node){
    char op_code = unary_node->getOpCode();

    unary_node->operandAccept(this);
    llvm::Value * operand_val = popLlvmValue();

    llvm::Value * unary_val;
    switch(op_code){
    case('-'):{
        if (unary_node->getType() == type_double){
            unary_val = builder_->CreateFNeg(operand_val,"unary_fneg_temp");
        }
        else{
            unary_val = builder_->CreateNeg(operand_val,"unary_neg_temp");
        }
        break;
    }
    case('!'):{
        unary_val = builder_->CreateNot(operand_val,"unary_not_temp");
        break;
    }
    default:{
        spdlog::error("Unknown unary operator {0}",op_code);
        throw BError(); 
    }
    }
    pushLlvmValue(unary_val);
}

void CodeGenerator::binaryExprAction(BinaryExprAST * binary_node){
    char op_code = binary_node->getOpCode();
    BType res_type = binary_node->getType();
    BType lhs_type = binary_node->getLHS().getType();
    BType rhs_type = binary_node->getRHS().getType();

    binary_node->lhsAccept(this);
    llvm::Value * lhs_val = popLlvmValue();
    binary_node->rhsAccept(this);
    llvm::Value * rhs_val = popLlvmValue();

    spdlog::debug("doing a binop at {0}", binary_node->getLocStr());

    // Can replace this with more advanced function with lookup for
    // which types to use if more types or operators added.
    spdlog::debug("Checking if casts for binop needed");
    if(res_type == type_int){
        spdlog::debug("try doubles to ints");
        lhs_val = tryDoubleToIntCast(lhs_val);
        rhs_val = tryDoubleToIntCast(rhs_val);
    }
    else if(res_type == type_double){
        spdlog::debug("try int to doubbles");
        lhs_val = tryIntToDoubleCast(lhs_val);
        rhs_val = tryIntToDoubleCast(rhs_val);
    }
    else{
        spdlog::debug("Bool binary");
    }

    llvm::Value * binary_val;
    switch(op_code){
    case('+'):{
        binary_val = createAdd(res_type, lhs_val, rhs_val);
        break;
    }
    case('-'):{
        binary_val = createSub(res_type, lhs_val, rhs_val);
        break;
    }
    case('*'):{
        binary_val = createMul(res_type, lhs_val, rhs_val);
        break;
    }
    case('/'):{
        binary_val = createDiv(res_type, lhs_val, rhs_val);
        break;
    }
    case('<'):{
        binary_val = createLessThan(lhs_type, rhs_type, lhs_val, rhs_val);
        break;
    }
    case('>'):{
        binary_val = createGreaterThan(lhs_type, rhs_type, lhs_val, rhs_val);
        break;
    }
    case('&'):{
        binary_val = createAnd(res_type, lhs_val, rhs_val);
        break;
    }
    case('|'):{
        binary_val = createOr(res_type, lhs_val, rhs_val);
        break;
    }
    default:{
        spdlog::error("Unknown binary operator {0}",op_code);
        throw BError(); 
    }
    }
    spdlog::debug("Pushing binop");
    pushLlvmValue(binary_val);
}


void CodeGenerator::ifStAction(IfStatementAST * if_node){
    if_node->condAccept(this);
    llvm::Value * if_val = popLlvmValue();
    // check it is bool?

    llvm::Function * parent_function = builder_->GetInsertBlock()->getParent();

    // Passing parent_function also inserts the then block at the end of the function.
    llvm::BasicBlock * then_block = llvm::BasicBlock::Create(*context_,"then",parent_function); 
    llvm::BasicBlock * else_block = llvm::BasicBlock::Create(*context_,"else");
    llvm::BasicBlock * merge_block = llvm::BasicBlock::Create(*context_,"if_continue");

    // Branch to else block if it exists, else jump straight to merge.
    builder_->CreateCondBr(if_val, then_block, if_node->getHasElse()? else_block : merge_block);

    // Move, emit code, and branch to the merge block
    builder_->SetInsertPoint(then_block);
    if_node->thenAccept(this); 
    llvm::Instruction * term_inst = builder_->GetInsertBlock()->getTerminator();
    if(!term_inst){
        builder_->CreateBr(merge_block);
    }   
    
    if(if_node->getHasElse()){
        // Add else block to end of function and start inserting there.
        parent_function->getBasicBlockList().push_back(else_block);
        builder_->SetInsertPoint(else_block);

        if_node->elseAccept(this);
        term_inst = builder_->GetInsertBlock()->getTerminator();
        // If last instruction was a return (term_inst not null) then don't add second terminator
        if(!term_inst){
            builder_->CreateBr(merge_block);
        }
    }

    parent_function->getBasicBlockList().push_back(merge_block);
    builder_->SetInsertPoint(merge_block);
}

void CodeGenerator::forStAction(ForStatementAST * for_node){
    // initialise any induction variables etc.

    llvm::Function * parent_function = builder_->GetInsertBlock()->getParent();
    // Remember previous block as flow could come from here...
    // Unimportant as don't need phi nodes?
    llvm::BasicBlock * previous_block = builder_->GetInsertBlock();
    llvm::BasicBlock * loop_start_block = llvm::BasicBlock::Create(*context_, "loop_start", parent_function);
    llvm::BasicBlock * loop_cond_block = llvm::BasicBlock::Create(*context_, "loop_condition");
    llvm::BasicBlock * loop_body_block = llvm::BasicBlock::Create(*context_, "loop_body");
    llvm::BasicBlock * loop_end_block = llvm::BasicBlock::Create(*context_, "loop_end");

    builder_->CreateBr(loop_start_block); // explicit fall through to start
    builder_->SetInsertPoint(loop_start_block);
    for_node->startAccept(this);
    builder_->CreateBr(loop_cond_block);
    parent_function->getBasicBlockList().push_back(loop_cond_block);
    builder_->SetInsertPoint(loop_cond_block);

    for_node->endAccept(this);
    llvm::Value * cond_val = popLlvmValue();
    // While the condition is true, branch to the loop body, else skip
    builder_->CreateCondBr(cond_val, loop_body_block, loop_end_block);
    
    // Insert the body and step code
    parent_function->getBasicBlockList().push_back(loop_body_block);
    builder_->SetInsertPoint(loop_body_block);
    for_node->bodyAccept(this);
    const llvm::Instruction * term_inst = builder_->GetInsertBlock()->getTerminator();
    // If return is the last instruction in body, (thus not allowing loops)
    // then we don't emit the step and branch code.
    if(!term_inst){
        for_node->stepAccept(this);
        // Check condition for another iteration
        builder_->CreateBr(loop_cond_block);
    }    

    // Add body to end of function

    // TODO: handle scoping of induction variables?
    // Add loop end at end of function
    parent_function->getBasicBlockList().push_back(loop_end_block);
    builder_->SetInsertPoint(loop_end_block);
}

void CodeGenerator::whileStAction(WhileStatementAST * while_node){
    llvm::Function * parent_function = builder_->GetInsertBlock()->getParent();

    llvm::BasicBlock * loop_cond = llvm::BasicBlock::Create(*context_,"while_cond",parent_function);
    llvm::BasicBlock * loop_body = llvm::BasicBlock::Create(*context_,"while_body");
    llvm::BasicBlock * loop_end = llvm::BasicBlock::Create(*context_,"while_end");

    builder_->CreateBr(loop_cond);

    builder_->SetInsertPoint(loop_cond);
    while_node->condAccept(this);
    llvm::Value * cond_val = popLlvmValue();
    builder_->CreateCondBr(cond_val,loop_body,loop_end);
    
    parent_function->getBasicBlockList().push_back(loop_body);
    builder_->SetInsertPoint(loop_body);
    while_node->bodyAccept(this);
    llvm::Instruction * term_inst = builder_->GetInsertBlock()->getTerminator();
    // only add branch back if not already terminated with a return
    if(!term_inst){
        builder_->CreateBr(loop_cond);
    }

    parent_function->getBasicBlockList().push_back(loop_end);
    builder_->SetInsertPoint(loop_end);
}

void CodeGenerator::returnStAction(ReturnStatementAST * return_node){
    // codegen return value
    return_node->returnExprAccept(this);
    llvm::Value * return_val = popLlvmValue();
    builder_->CreateRet(return_val);
}

void CodeGenerator::blockStAction(BlockStatementAST * block_node){
    int llvm_val_stack_size = llvm_value_stack_.size();

    // codegen for each statement, starting from this entry block
    block_node->resetStatementIndex();
    while(block_node->anotherStatement()){
        block_node->statementAcceptOne(this);
    }

    if(llvm_val_stack_size != llvm_value_stack_.size()){
        spdlog::error("Value stack size not maintained in block");
        throw BError();
    }
}

void CodeGenerator::callStAction(CallStatementAST * call_node){
    call_node->callAccept(this);
    popLlvmValue();
    //llvm::Value * call_val = popLlvmValue();
    //pushLlvmValue(call_val); // call statement with no capture, can discard the value.
}

void CodeGenerator::assignStAction(AssignStatementAST * assign_node){
    assign_node->valueAccept(this);
    llvm::Value * val_to_assign = popLlvmValue();

    std::string var_name = assign_node->getIdentifier();

    // TODO ensure that var is already initialised.
    llvm::AllocaInst * alloca = named_values_[var_name];

    // TYPE CHECK
    BType dest_type = assign_node->getDestType();
    if(convertBType(dest_type) != val_to_assign->getType()){
        val_to_assign = createCast(val_to_assign,dest_type);
    }
    builder_->CreateStore(val_to_assign, alloca);
}

void CodeGenerator::initStAction(InitStatementAST * init_node){
    //TODO validate that not overwriting.
    std::string var_name = init_node->getIdentifier();
    spdlog::debug("generating init statement");
    llvm::Type * var_type = convertBType(init_node->getType());
    llvm::AllocaInst *alloca = builder_->CreateAlloca(var_type, nullptr, var_name);
    named_values_[var_name] = alloca;

    init_node->assignmentAccept(this);
}

void CodeGenerator::prototypeAction(PrototypeAST * proto_node){
    std::vector<llvm::Type *> llvm_arg_types;
    BFType b_func_type = proto_node->getType();
    auto b_arg_types = b_func_type.getArgumentTypes();
    for (auto b_arg_type:b_arg_types){
        spdlog::debug("generating prototype args");
        llvm_arg_types.push_back(convertBType(b_arg_type));
    }

    spdlog::debug("generating prototype RET");
    auto ret_type = convertBType(b_func_type.getReturnType());
    llvm::FunctionType * func_type = llvm::FunctionType::get(ret_type, llvm_arg_types, false);

    llvm::Function * func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, proto_node->getName(), module_.get());

    std::vector<std::pair<std::string,BType>> args = proto_node->getArgs();
    unsigned i = 0;
    for(auto &arg : func->args()){
        arg.setName(args[i++].first);
    }

    pushLlvmProto(func);
}

void CodeGenerator::functionAction(FunctionAST * func_node){
    llvm::Function * function = module_->getFunction(func_node->getProto().getName());

    if(!function){
        spdlog::debug("Function not previously declared, accepting proto");
        func_node->protoAccept(this);
        function = popLlvmProto();
    }

    // TODO VERIFY THAT FUNCTION MATCHES func_node's signature.

    if(!function->empty()){
        spdlog::debug("Expected function to be empty at definition");
        throw BError();
    }

    llvm::BasicBlock *entry_block = llvm::BasicBlock::Create(*context_, "entry", function);
    builder_->SetInsertPoint(entry_block);

    // If args exist, record them in named values here
    named_values_.clear();
    for(auto &arg : function->args()){
        std::string arg_name = arg.getName().str();
        llvm::AllocaInst *alloca = createEntryBlockAlloca(function, &arg);
        builder_->CreateStore(&arg, alloca);
        named_values_[arg_name] = alloca;
    }

    llvm::BasicBlock *body_block = llvm::BasicBlock::Create(*context_, "body", function);
    builder_->CreateBr(body_block);
    builder_->SetInsertPoint(body_block);

    func_node->bodyAccept(this);

    // Handle void functions
    llvm::Instruction * term_inst = builder_->GetInsertBlock()->getTerminator();
    if(!term_inst && func_node->getType().getReturnType()==type_void){
        builder_->CreateRetVoid();
    }

    // If we catch an error in the above accepts, then erase this function from parent
    // function->eraseFromParent();

    std::error_code EC;

    if(llvm::verifyFunction(*function)){
        // true indicates errors encountered.
        this->printIR();
        spdlog::error("function {0}: body not verified", func_node->getProto().getName());
        throw BError();
    }
    
}

void CodeGenerator::topLevelsAction(TopLevels * top_levels_node){
    // setup the main function
    llvm::FunctionType * func_type = llvm::FunctionType::get(llvm::Type::getInt32Ty(*context_), false); 
    llvm::Function * main_function = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "main", module_.get());

    llvm::BasicBlock *main_entry_block = llvm::BasicBlock::Create(*context_, "main_entry", main_function);
    builder_->SetInsertPoint(main_entry_block);

    // Add to a basic block inside the function
    top_levels_node->statementsAllAccept(this);

    llvm::Value * pass_val = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_),0,true);
    builder_->CreateRet(pass_val);

    if(llvm::verifyFunction(*main_function)){
        // true indicates errors encountered.
        spdlog::error("Main not verified.");
        throw BError();
    }
}

void CodeGenerator::funcDefsAction(FuncDefs * func_defs_node){
    func_defs_node->functionsAllAccept(this);
}
void CodeGenerator::programAction(BProgram * program_node){
    program_node->funcDefsAccept(this);
    spdlog::info("Functions Generated");
    program_node->topLevelsAccept(this);
    spdlog::info("Top Level Statements Generated ");
    spdlog::debug("Final stack size {0:d}", llvm_value_stack_.size());
}

} // namespace codegen
} // namespace bassoon