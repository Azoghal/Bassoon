#ifndef Bassoon_include_codegen_HXX
#define Bassoon_include_codegen_HXX

#include <map>
#include "ast.hxx"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FileSystem.h"

#include "llvm/Support/Casting.h"


namespace bassoon{
namespace codegen{

class CodeGenerator : public ASTVisitor {
    std::unique_ptr<llvm::LLVMContext> context_;
    std::unique_ptr<llvm::Module> module_;
    std::unique_ptr<llvm::IRBuilder<>> builder_;
    std::map<std::string, llvm::AllocaInst *> named_values_;

    llvm::TargetMachine * target_machine_;

    std::vector<llvm::Value *> llvm_value_stack_;
    std::vector<llvm::Function *> llvm_function_stack_;

    llvm::Value * popLlvmValue();
    llvm::Function * popLlvmFunction();
    llvm::Type * convertBType(BType btype);

    llvm::AllocaInst * createEntryBlockAlloca(llvm::Function *function, std::string var_name);

    llvm::Value * createAdd(BType res_type, llvm::Value * lhs_val, llvm::Value * rhs_val);
    llvm::Value * createSub(BType res_type, llvm::Value * lhs_val, llvm::Value * rhs_val);
    llvm::Value * createMul(BType res_type, llvm::Value * lhs_val, llvm::Value * rhs_val);
    llvm::Value * createDiv(BType res_type, llvm::Value * lhs_val, llvm::Value * rhs_val);
public:
    CodeGenerator();
    void SetTarget();
    void MakeTestIR();
    void MakeTestMainIR();
    void DefinePutChar();
    void PrintIR();
    void Compile();
    void Generate(std::shared_ptr<BProgram> program);

    void boolExprAction(BoolExprAST * bool_node) override;
    void intExprAction(IntExprAST * int_node) override;
    void doubleExprAction(DoubleExprAST * double_node) override;
    void variableExprAction(VariableExprAST * variable_node) override;
    void callExprAction(CallExprAST * call_node) override;
    void unaryExprAction(UnaryExprAST * unary_node) override;
    void binaryExprAction(BinaryExprAST * binary_node) override;
    
    void ifStAction(IfStatementAST * if_node) override;
    void forStAction(ForStatementAST * for_node) override;
    void whileStAction(WhileStatementAST * while_node) override;
    void returnStAction(ReturnStatementAST * return_node) override;
    void blockStAction(BlockStatementAST * block_node) override;
    void callStAction(CallStatementAST * call_node) override;
    void assignStAction(AssignStatementAST * assign_node) override;
    void initStAction(InitStatementAST * init_node) override;

    void prototypeAction(PrototypeAST * proto_node) override;
    void functionAction(FunctionAST * func_node) override;

    void topLevelsAction(TopLevels * top_levels_node) override;
    void funcDefsAction(FuncDefs * func_defs_node) override;
    void programAction(BProgram * program_node) override;
};

} // namespace codegen
} // namespace bassoon

#endif // Bassoon_include_codegen_HXX