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

void CodeGenerator::SetTarget(){
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

llvm::AllocaInst * CodeGenerator::createEntryBlockAlloca(llvm::Function *function, std::string var_name){
    // Make an IR builder pointing to first instruction of entry block.
    llvm::IRBuilder<> temp_builder (&function->getEntryBlock(), function->getEntryBlock().begin());
    return temp_builder.CreateAlloca(llvm::Type::getDoubleTy(*context_),0,var_name.c_str());
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
    llvm::Value * val = llvm_value_stack_[llvm_value_stack_.size()-1];
    llvm_value_stack_.pop_back();
    return val;
}

llvm::Function * CodeGenerator::popLlvmFunction(){
    llvm::Function * f = llvm_function_stack_[llvm_function_stack_.size()-1];
    llvm_function_stack_.pop_back();
    return f;
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
        return builder_->CreateSDiv(lhs_val,rhs_val,"int_bin_div_temp");
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

// void CodeGenerator::MakeTestIR(){
//     // Test Func is f(x) = x+5;
//     // nullptr return type - void. not var args
//     std::vector<llvm::Type*> arg_types;
//     arg_types.push_back(llvm::Type::getInt32Ty(*context_));
//     llvm::FunctionType * func_type = llvm::FunctionType::get(llvm::Type::getInt32Ty(*context_), arg_types, false); 
//     llvm::Function * test_func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "testFun", module_.get());

//     // Set the arg names
//     std::vector<std::string> arg_names = {"x"};
//     unsigned i = 0;
//     for(auto &arg : test_func->args()){
//         arg.setName(arg_names[i++]);
//     }


//     // Make sure not already defined
//     if(module_->getFunction("testFun")){
//         fprintf(stderr,"testFun already defined\n");
//     }

//     // Body
//     llvm::BasicBlock *BB = llvm::BasicBlock::Create(*context_, "entry", test_func);
//     builder_->SetInsertPoint(BB);

//     // If args exist, record them in named values here
//     named_values_.clear();
//     for(auto &arg : test_func->args()){
//         std::string arg_name = arg.getName().str();
//         named_values_[arg_name] = &arg;
//     }

//     const int val = 5;
//     llvm::Value * L = named_values_["x"];
//     llvm::Value * R = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_), 5, true);
//     llvm::Value * sum = builder_->CreateAdd(L,R,"addTemp");
//     builder_->CreateRet(sum); 
// }

// void CodeGenerator::MakeTestMainIR(){
//     // Test Func is f(x) = x+5;
//     // nullptr return type - void. not var args
//     fprintf(stderr,"First");
//     llvm::FunctionType * func_type = llvm::FunctionType::get(llvm::Type::getVoidTy(*context_), false); 
//     fprintf(stderr,"Second");
//     llvm::Function * test_func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "main", module_.get());
//     fprintf(stderr,"Third");

//     // Make sure not already defined
//     if(module_->getFunction("main")){
//         fprintf(stderr,"main already defined\n");
//     }

//     // Body
//     llvm::BasicBlock *BB = llvm::BasicBlock::Create(*context_, "entry", test_func);
//     builder_->SetInsertPoint(BB);

//     std::vector<llvm::Value *> args;
//     llvm::Value * arg_val = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_), 'a', true);
//     // llvm::Value * arg_val_ptr = llvm::IntToPtrInst(arg_val,llvm::Type::getInt8PtrTy(*context_),)
//     args.push_back(arg_val);
//     builder_->CreateCall(module_->getFunction("putchar"),args);
    

//     const int val = 0;
//     llvm::Value * result = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_), val, true);
//     builder_->CreateRet(result); 
// }

void CodeGenerator::DefinePutChar(){
    std::vector<llvm::Type *> arg_types;
    arg_types.push_back(llvm::Type::getInt32Ty(*context_));
    llvm::FunctionType * func_type = llvm::FunctionType::get(llvm::Type::getInt32Ty(*context_), arg_types, false); 
    llvm::Function * func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "putchar", module_.get());
}

void CodeGenerator::PrintIR(){
    module_->print(llvm::errs(), nullptr);
}

void CodeGenerator::Compile(){
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

    fprintf(stderr,"running the module\n");

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
    fprintf(stderr,"making int value %i \n", int_node->getValue());
    llvm::Value * int_const = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_),int_node->getValue(),true);
    if (llvm::ConstantInt* CI = llvm::dyn_cast<llvm::ConstantInt>(int_const)) {
        // foo indeed is a ConstantInt, we can use CI here
        if (CI->getBitWidth() <= 32) {
            int int_val = CI->getSExtValue();
            fprintf(stderr,"%i\n",int_val);
        }
    }
    else {
        // foo was not actually a ConstantInt
    }
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

    // TODO Check number of args ...

    // codegen the args and pop them off into args vec
    std::vector<llvm::Value *> args_vec;
    while(call_node->anotherArg()){
        call_node->argAcceptOne(this);
        args_vec.push_back(popLlvmValue());
    }

    llvm::Value * call_val = builder_->CreateCall(callee_func,args_vec,"calltmp");
}

void CodeGenerator::unaryExprAction(UnaryExprAST * unary_node){
    char op_code = unary_node->getOpCode();
    // BType type = unary_node->getType();

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
    BType res_type = binary_node->getType();

    binary_node->lhsAccept(this);
    binary_node->rhsAccept(this);
    llvm::Value * rhs_val = popLlvmValue();
    llvm::Value * lhs_val = popLlvmValue();

    llvm::Value * binary_val;
    switch(op_code){
    case('+'):{
        createAdd(res_type, lhs_val, rhs_val);
        break;
    }
    case('-'):{
        createSub(res_type, lhs_val, rhs_val);
        break;
    }
    case('*'):{
        createMul(res_type, lhs_val, rhs_val);
        break;
    }
    case('/'):{
        createDiv(res_type, lhs_val, rhs_val);
        break;
    }
    default:{
        fprintf(stderr,"Unknown binary operator %c\n",op_code);
        throw BError(); 
    }
    }
    llvm_value_stack_.push_back(binary_val);
}


void CodeGenerator::ifStAction(IfStatementAST * if_node){}
void CodeGenerator::forStAction(ForStatementAST * for_node){}
void CodeGenerator::whileStAction(WhileStatementAST * while_node){}

void CodeGenerator::returnStAction(ReturnStatementAST * return_node){
    // codegen return value
    fprintf(stderr,"codegening return\n");
    return_node->returnExprAccept(this);
    llvm::Value * return_val = popLlvmValue();
    fprintf(stderr,"Making return");
    llvm::Value * ret = builder_->CreateRet(return_val);
    llvm_value_stack_.push_back(ret);

    module_->print(llvm::errs(), nullptr);
}

void CodeGenerator::blockStAction(BlockStatementAST * block_node){
    llvm::BasicBlock *block_statement_entry = llvm::BasicBlock::Create(*context_, "block_statement_entry");
    builder_->SetInsertPoint(block_statement_entry);

    int llvm_val_stack_size = llvm_value_stack_.size();

    // codegen for each statement, starting from this entry block
    // need to call all the statements... and make sure they're popped off.
    block_node->resetStatementIndex();
    while(block_node->anotherStatement()){
        block_node->statementAcceptOne(this);
        fprintf(stderr,"popping and discarding a statement in blockstaction\n");
        popLlvmValue(); // discard the pushed value.
    }

    if(llvm_val_stack_size != llvm_value_stack_.size()){
        fprintf(stderr,"Value stack size not maintained in block\n");
        throw BError();
    }

    llvm_value_stack_.push_back(block_statement_entry);
}

void CodeGenerator::callStAction(CallStatementAST * call_node){
    call_node->callAccept(this);
    // llvm::Value * call_val = popLlvmValue();
    // llvm_value_stack_.push_back(call_val);
}

void CodeGenerator::assignStAction(AssignStatementAST * assign_node){}
void CodeGenerator::initStAction(InitStatementAST * init_node){}

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

    llvm_function_stack_.push_back(func);
}

void CodeGenerator::functionAction(FunctionAST * func_node){
    fprintf(stderr,"function action\n");
    llvm::Function * function = module_->getFunction(func_node->getProto().getName());

    if(!function){
        fprintf(stderr,"Function not previously declared, accepting proto\n");
        func_node->protoAccept(this);
        function = popLlvmFunction();
    }

    // TODO VERIFY THAT FUNCTION MATCHES func_node's signature.

    if(!function->empty()){
        fprintf(stderr,"Expected function to be empty at definition\n");
        throw BError();
    }

    fprintf(stderr,"making basic block\n");
    llvm::BasicBlock *BB = llvm::BasicBlock::Create(*context_, "entry", function);
    builder_->SetInsertPoint(BB);

    // If args exist, record them in named values here
    named_values_.clear();
    for(auto &arg : function->args()){
        std::string arg_name = arg.getName().str();
        llvm::AllocaInst *alloca = createEntryBlockAlloca(function, arg_name);
        builder_->CreateStore(&arg, alloca);
        named_values_[arg_name] = alloca;
    }

    fprintf(stderr,"codegening body\n");
    func_node->bodyAccept(this);
    llvm::Value * body_ret_val = popLlvmValue();

    // If we catch an error in the above accept, then erase this function from parent
    // function->eraseFromParent();

    fprintf(stderr,"verifying the function\n");
    if(!llvm::verifyFunction(*function)){
        fprintf(stderr,"function body not verified.\n");
    }

    fprintf(stderr,"pushing the function\n");
    llvm_function_stack_.push_back(function);
}

void CodeGenerator::topLevelsAction(TopLevels * top_levels_node){
    fprintf(stderr,"Actually doing Top Levels defs\n");
    // setup an anonymous void function with no arguments containing the top level statements.
    // llvm::FunctionType * func_type = llvm::FunctionType::get(llvm::Type::getVoidTy(*context_), false); 
    // llvm::Function * main_function = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "__main__", module_.get());

    llvm::BasicBlock *main_entry_block = llvm::BasicBlock::Create(*context_, "main_entry");
    builder_->SetInsertPoint(main_entry_block);

    // Add to a basic block inside the function
    top_levels_node->statementsAllAccept(this);
}

void CodeGenerator::funcDefsAction(FuncDefs * func_defs_node){
    fprintf(stderr,"Actually doing func defs, should be %i\n", func_defs_node->countFuncs());
    func_defs_node->functionsAllAccept(this);
}
void CodeGenerator::programAction(BProgram * program_node){
    program_node->funcDefsAccept(this);
    fprintf(stderr,"finished with funcdefs\n");
    program_node->topLevelsAccept(this);
    fprintf(stderr,"finished with toplevels\n");

}

} // namespace codegen
} // namespace bassoon