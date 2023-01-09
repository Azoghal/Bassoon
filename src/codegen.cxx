#include "ast.hxx"
#include "codegen.hxx"
#include "exceptions.hxx"

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

void CodeGenerator::MakeTestIR(){
    // Test Func is f(x) = x+5;
    // nullptr return type - void. not var args
    std::vector<llvm::Type*> arg_types;
    arg_types.push_back(llvm::Type::getInt32Ty(*context_));
    llvm::FunctionType * func_type = llvm::FunctionType::get(llvm::Type::getInt32Ty(*context_), arg_types, false); 
    llvm::Function * test_func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "testFun", module_.get());

    // Set the arg names
    std::vector<std::string> arg_names = {"x"};
    unsigned i = 0;
    for(auto &arg : test_func->args()){
        arg.setName(arg_names[i++]);
    }


    // Make sure not already defined
    if(module_->getFunction("testFun")){
        fprintf(stderr,"testFun already defined\n");
    }

    // Body
    llvm::BasicBlock *BB = llvm::BasicBlock::Create(*context_, "entry", test_func);
    builder_->SetInsertPoint(BB);

    // If args exist, record them in named values here
    named_values_.clear();
    for(auto &arg : test_func->args()){
        std::string arg_name = arg.getName().str();
        named_values_[arg_name] = &arg;
    }

    const int val = 5;
    llvm::Value * L = named_values_["x"];
    llvm::Value * R = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_), 5, true);
    llvm::Value * sum = builder_->CreateAdd(L,R,"addTemp");
    builder_->CreateRet(sum); 
}

void CodeGenerator::MakeTestMainIR(){
    // Test Func is f(x) = x+5;
    // nullptr return type - void. not var args
    fprintf(stderr,"First");
    llvm::FunctionType * func_type = llvm::FunctionType::get(llvm::Type::getVoidTy(*context_), false); 
    fprintf(stderr,"Second");
    llvm::Function * test_func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "main", module_.get());
    fprintf(stderr,"Third");

    // Make sure not already defined
    if(module_->getFunction("main")){
        fprintf(stderr,"main already defined\n");
    }

    // Body
    llvm::BasicBlock *BB = llvm::BasicBlock::Create(*context_, "entry", test_func);
    builder_->SetInsertPoint(BB);


    const int val = 0;
    llvm::Value * result = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_), val, true);
    builder_->CreateRet(result); 
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
    pass_manager.run(*module_);
    destination.flush();
}

} // namespace codegen
} // namespace bassoon