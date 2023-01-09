#ifndef Bassoon_include_codegen_HXX
#define Bassoon_include_codegen_HXX

#include <map>
#include "ast.hxx"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FileSystem.h"

namespace bassoon{
namespace codegen{

class CodeGenerator{ //:public ASTVisitor
    std::unique_ptr<llvm::LLVMContext> context_;
    std::unique_ptr<llvm::Module> module_;
    std::unique_ptr<llvm::IRBuilder<>> builder_;
    std::map<std::string, llvm::Value *> named_values_;

    llvm::TargetMachine * target_machine_;
public:
    CodeGenerator();
    void SetTarget();
    void MakeTestIR();
    void MakeTestMainIR();
    void PrintIR();
    void Compile();
};

} // namespace codegen
} // namespace bassoon

#endif // Bassoon_include_codegen_HXX