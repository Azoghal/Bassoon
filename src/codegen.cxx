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
    
}

void CodeGenerator::PrintIR(){
    module_->print(llvm::errs(), nullptr);
}

} // namespace codegen
} // namespace bassoon