#include "ast.hxx"
#include "codegen.hxx"

namespace bassoon
{
namespace codegen
{

CodeGenerator::CodeGenerator(){
    context_ = std::make_unique<llvm::LLVMContext>();
    module_ = std::make_unique<llvm::Module>("doubleReed", *context_);
    builder_ = std::make_unique<llvm::IRBuilder<>>(*context_);
}

void CodeGenerator::MakeTestIR(){
    // Test Func is f(x) = x+5;
    // nullptr return type - void. not var args
    std::vector<llvm::Type*> arg_types;
    arg_types.push_back(llvm::Type::getInt32Ty(*context_));
    llvm::FunctionType * func_type = llvm::FunctionType::get(llvm::Type::getInt32Ty(*context_), arg_types, false);
    llvm::Function * test_func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "testFunc", module_.get());

    // Set the arg names
    std::vector<std::string> arg_names = {"x"};
    unsigned i = 0;
    for(auto &arg : test_func->args()){
        arg.setName(arg_names[i++]);
    }

    // Make sure not already defined
    if(module_->getFunction("testFunc")){
        fprintf(stderr,"testFunc already defined");
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
    
}

void CodeGenerator::PrintIR(){
    module_->print(llvm::errs(), nullptr);
}

} // namespace codegen
} // namespace bassoon