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

void CodeGenerator::generate(std::shared_ptr<BProgram> program){
    program->accept(this);
}

void CodeGenerator::setTarget(){
    std::string target_triple = llvm::sys::getDefaultTargetTriple();
    if(target_triple != "x86_64-unknown-linux-gnu"){
        fprintf(stderr,"Initialising all targets\n");
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
        fprintf(stderr,"%s\n", target_lookup_error.c_str());
        return;
    }
    
    std::string cpu = "generic";
    std::string features = "";
    llvm::TargetOptions options;
    auto relocation_model = llvm::Optional<llvm::Reloc::Model>();
    target_machine_ = target->createTargetMachine(target_triple, cpu, features, options, relocation_model);
    module_->setDataLayout(target_machine_->createDataLayout());
    module_->setTargetTriple(target_triple);
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
        fprintf(stderr,"not a convertible type %s", typeToStr(btype).c_str());
        throw BError();
    }
    }
}

llvm::Value * CodeGenerator::popLlvmValue(){
    if (!(llvm_value_stack_.size()>0)){
        fprintf(stderr, "llvm_value_stack is empty but a pop was attempted\n");
        throw BError();
    }
    llvm::Value * val = llvm_value_stack_[llvm_value_stack_.size()-1];
    llvm_value_stack_.pop_back();
    fprintf(stderr,"POPPED, size: %lu\n",llvm_value_stack_.size());
    return val;
}

llvm::Function * CodeGenerator::popLlvmProto(){
    if (!(llvm_proto_stack_.size()>0)){
        fprintf(stderr, "llvm_proto_stack is empty but a pop was attempted\n");
        throw BError();
    }
    llvm::Function * f = llvm_proto_stack_[llvm_proto_stack_.size()-1];
    llvm_proto_stack_.pop_back();
    return f;
}

//------------------------
// Builder Helpers
//------------------------

llvm::Value * CodeGenerator::createAdd(BType res_type, llvm::Value * lhs_val, llvm::Value * rhs_val){
    fprintf(stderr,"Making an add\n");
    switch(res_type){
    case(type_int):{
        fprintf(stderr,"int add\n");
        return builder_->CreateAdd(lhs_val,rhs_val,"int_bin_add_temp");
    }
    case(type_double):{
        fprintf(stderr,"double add\n");
        return builder_->CreateFAdd(lhs_val,rhs_val,"double_bin_add_temp");
    }
    default:{
        fprintf(stderr, "Type is not a valid result of sum operation\n");
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
        fprintf(stderr, "Type is not a valid result of sub operation\n");
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
        fprintf(stderr, "Type is not a valid result of mul operation\n");
        throw BError();
    }
    }
}

llvm::Value * CodeGenerator::createDiv(BType res_type, llvm::Value * lhs_val, llvm::Value * rhs_val){
    switch(res_type){
    case(type_int):{
        builder_->CreateSDiv(lhs_val,rhs_val,"int_bin_div_temp");
    }
    case(type_double):{
        return builder_->CreateFDiv(lhs_val,rhs_val,"double_bin_div_temp");
    }
    default:{
        fprintf(stderr, "Type is not a valid result of div operation\n");
        throw BError();
    }
    }
}

llvm::Value * CodeGenerator::createLessThan(BType lhs_type, BType rhs_type, llvm::Value * lhs_val, llvm::Value * rhs_val){
    switch(lhs_type){
    case(type_int):{
        switch(rhs_type){
        case(type_int):{
            // No cast required, integer comparison
            return builder_->CreateICmpSLT(lhs_val, rhs_val, "int_cmp");
            break;
        }
        case(type_double):{
            // need to cast lhs to double for comparison
            llvm::Value * lhs_double = builder_->CreateIntCast(lhs_val,convertBType(type_double), true, "int_to_double_cast");
            return builder_->CreateFCmpOLT(lhs_double,rhs_val,"double_cmp");
            break;
        }
        default:break;
        }
        break;
    }
    case(type_double):{
        switch(rhs_type){
        case(type_int):{
            // need to cast rhs to double
            llvm::Value * rhs_double = builder_->CreateIntCast(rhs_val,convertBType(type_double), true, "int_to_double_cast");
            return builder_->CreateFCmpOLT(lhs_val,rhs_double,"double_cmp");
        }
        case(type_double):{
            // no cast needed
            return builder_->CreateFCmpOLT(lhs_val, rhs_val, "double_cmp");
        }
        default:break;
        }
    }
    default:break;
    }
    fprintf(stderr," < used on non numerical type\n");
    throw BError();
}

void CodeGenerator::definePutChar(){
    std::vector<llvm::Type *> arg_types;
    arg_types.push_back(llvm::Type::getInt32Ty(*context_));
    llvm::FunctionType * func_type = llvm::FunctionType::get(llvm::Type::getInt32Ty(*context_), arg_types, false); 
    llvm::Function * func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "putchar", module_.get());
}

void CodeGenerator::printIR(){
    module_->print(llvm::errs(), nullptr);
}

void CodeGenerator::compile(){
    std::string object_filename = "output.o";
    std::error_code EC;
    llvm::raw_fd_ostream destination (object_filename, EC, llvm::sys::fs::OF_None);
    if(EC){
        llvm::errs() << "Can't open file " << EC.message();
        return;
    }
    llvm::legacy::PassManager pass_manager;
    auto file_type = llvm::CGFT_ObjectFile; // code gen file type

    if(target_machine_->addPassesToEmitFile(pass_manager, destination, nullptr, file_type)){
        llvm::errs() << "target machine can't emit a file of this type";
        return;
    }

    pass_manager.run(*module_);
    destination.flush();
}

//--------------------
// Expression Actions
//--------------------

void CodeGenerator::boolExprAction(BoolExprAST * bool_node){
    llvm::Value * bool_const = llvm::ConstantInt::get(llvm::Type::getInt1Ty(*context_),bool_node->getValue(),false);
    llvm_value_stack_.push_back(bool_const);
}

void CodeGenerator::intExprAction(IntExprAST * int_node){
    llvm::Value * int_const = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_),int_node->getValue(),true);
    llvm_value_stack_.push_back(int_const);
}

void CodeGenerator::doubleExprAction(DoubleExprAST * double_node){
    llvm::Value * double_const = llvm::ConstantFP::get(*context_,llvm::APFloat(double_node->getValue()));
    llvm_value_stack_.push_back(double_const);
}

void CodeGenerator::variableExprAction(VariableExprAST * variable_node){
    std::string name = variable_node->getName();
    std::string loc_str = variable_node->getLocStr();
    llvm::Type * llvm_type = convertBType(variable_node->getType());
    llvm::Value *var_val = named_values_[variable_node->getName()];
    if(!var_val){
        fprintf(stderr,"Variable name unknown %s at %s",name.c_str(), loc_str.c_str());
        throw BError();
    }
    llvm::Value * load_val = builder_->CreateLoad(llvm_type, var_val, name.c_str());
    llvm_value_stack_.push_back(load_val);
}

void CodeGenerator::callExprAction(CallExprAST * call_node){
    llvm::Function * callee_func = module_->getFunction(call_node->getName());
    if(!callee_func){
        fprintf(stderr,"Unknown function called\n");
        throw BError();
    }

    // codegen the args and pop them off into args vec
    std::vector<llvm::Value *> args_vec;
    call_node->resetArgIndex();
    while(call_node->anotherArg()){
        call_node->argAcceptOne(this);
        llvm::Value * arg_val = popLlvmValue();
        args_vec.push_back(arg_val);
    }

    if(callee_func->arg_size() != args_vec.size()){
        fprintf(stderr,"mismatch arg size\n");
        throw BError();
    }

    llvm::Value * ret_val = builder_->CreateCall(callee_func,args_vec,"calltmp");
    llvm_value_stack_.push_back(ret_val);
}

void CodeGenerator::unaryExprAction(UnaryExprAST * unary_node){
    char op_code = unary_node->getOpCode();

    unary_node->operandAccept(this);
    llvm::Value * operand_val = popLlvmValue();

    llvm::Value * unary_val;
    switch(op_code){
    case('-'):{
        unary_val = builder_->CreateNeg(operand_val,"unary_neg_temp");
        break;
    }
    case('!'):{
        unary_val = builder_->CreateNot(operand_val,"unary_not_temp");
        break;
    }
    default:{
        fprintf(stderr,"Unknown unary operator %c\n",op_code);
        throw BError(); 
    }
    }
    llvm_value_stack_.push_back(unary_val);
}

void CodeGenerator::binaryExprAction(BinaryExprAST * binary_node){
    char op_code = binary_node->getOpCode();
    fprintf(stderr,"OPCODE: %c", op_code);
    BType res_type = binary_node->getType();
    BType lhs_type = binary_node->getLHS().getType();
    BType rhs_type = binary_node->getRHS().getType();

    binary_node->lhsAccept(this);
    llvm::Value * lhs_val = popLlvmValue();
    binary_node->rhsAccept(this);
    llvm::Value * rhs_val = popLlvmValue();

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
    default:{
        fprintf(stderr,"Unknown binary operator %c\n",op_code);
        throw BError(); 
    }
    }
    fprintf(stderr,"Finished binary makage\n");
    llvm_value_stack_.push_back(binary_val);
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

    builder_->CreateCondBr(if_val, then_block, else_block);

    // Move, emit code, and branch to the merge block
    builder_->SetInsertPoint(then_block);
    if_node->thenAccept(this); 
    builder_->CreateBr(merge_block);

    // get up to date then block, if e.g. nested blocks within then.
    then_block = builder_->GetInsertBlock();
    
    // Add else block to end of function and start inserting there.
    parent_function->getBasicBlockList().push_back(else_block);
    builder_->SetInsertPoint(else_block);

    if_node->elseAccept(this);
    builder_->CreateBr(merge_block);

    // get up to date else block.
    else_block = builder_->GetInsertBlock();

    parent_function->getBasicBlockList().push_back(merge_block);
    builder_->SetInsertPoint(merge_block);
    // do not need a phi node as everything handled with stores.
    //llvm::PHINode * phi = builder_->CreatePHI(...)

}

void CodeGenerator::forStAction(ForStatementAST * for_node){}
void CodeGenerator::whileStAction(WhileStatementAST * while_node){}

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
        fprintf(stderr,"Value stack size not maintained in block\n");
        throw BError();
    }
}

void CodeGenerator::callStAction(CallStatementAST * call_node){
    call_node->callAccept(this);
    popLlvmValue();
    //llvm::Value * call_val = popLlvmValue();
    //llvm_value_stack_.push_back(call_val); // call statement with no capture, can discard the value.
}

void CodeGenerator::assignStAction(AssignStatementAST * assign_node){
    assign_node->valueAccept(this);
    llvm::Value * val_to_assign = popLlvmValue();

    std::string var_name = assign_node->getIdentifier();

    // TODO ensure that var is already initialised.

    llvm::AllocaInst * alloca = named_values_[var_name];
    builder_->CreateStore(val_to_assign, alloca);
}

void CodeGenerator::initStAction(InitStatementAST * init_node){
    //TODO validate that not overwriting.
    std::string var_name = init_node->getIdentifier();
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
        llvm_arg_types.push_back(convertBType(b_arg_type));
    }

    auto ret_type = convertBType(b_func_type.getReturnType());
    llvm::FunctionType * func_type = llvm::FunctionType::get(ret_type, llvm_arg_types, false);

    llvm::Function * func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, proto_node->getName(), module_.get());

    std::vector<std::pair<std::string,BType>> args = proto_node->getArgs();
    unsigned i = 0;
    for(auto &arg : func->args()){
        arg.setName(args[i++].first);
    }

    llvm_proto_stack_.push_back(func);
}

void CodeGenerator::functionAction(FunctionAST * func_node){
    llvm::Function * function = module_->getFunction(func_node->getProto().getName());

    if(!function){
        fprintf(stderr,"Function not previously declared, accepting proto\n");
        func_node->protoAccept(this);
        function = popLlvmProto();
    }

    // TODO VERIFY THAT FUNCTION MATCHES func_node's signature.

    if(!function->empty()){
        fprintf(stderr,"Expected function to be empty at definition\n");
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

    // If we catch an error in the above accept, then erase this function from parent
    // function->eraseFromParent();

    fprintf(stderr,"Verifying function %s\n", func_node->getProto().getName().c_str());
    if(llvm::verifyFunction(*function)){
        // true indicates errors encountered.
        fprintf(stderr,"function body not verified.\n");
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

    fprintf(stderr,"Verifying main function\n");
    if(llvm::verifyFunction(*main_function)){
        // true indicates errors encountered.
        fprintf(stderr,"Main not verified.\n");
        throw BError();
    }
}

void CodeGenerator::funcDefsAction(FuncDefs * func_defs_node){
    func_defs_node->functionsAllAccept(this);
}
void CodeGenerator::programAction(BProgram * program_node){
    program_node->funcDefsAccept(this);
    fprintf(stderr,"Funcdefs COMPLETE\n");
    program_node->topLevelsAccept(this);
    fprintf(stderr,"Toplevels COMLETE \n");
    fprintf(stderr,"Final stack size %lu\n", llvm_value_stack_.size());
}

} // namespace codegen
} // namespace bassoon