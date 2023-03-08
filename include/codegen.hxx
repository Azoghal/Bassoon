#ifndef Bassoon_include_codegen_HXX
#define Bassoon_include_codegen_HXX

#include <map>
#include "ast.hxx"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/Analysis/CGSCCPassManager.h"
#include "llvm/Passes/PassBuilder.h"
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
    std::vector<llvm::Function *> llvm_proto_stack_;

    llvm::Value * popLlvmValue();
    void pushLlvmValue(llvm::Value * val);
    llvm::Function * popLlvmProto();
    void pushLlvmProto(llvm::Function * proto);
    llvm::Type * convertBType(BType btype);
    BType convertLlvmType(llvm::Type * type);

    llvm::AllocaInst * createEntryBlockAlloca(llvm::Function *function, llvm::Argument * arg);

    llvm::Value * createAdd(BType res_type, llvm::Value * lhs_val, llvm::Value * rhs_val);
    llvm::Value * createSub(BType res_type, llvm::Value * lhs_val, llvm::Value * rhs_val);
    llvm::Value * createMul(BType res_type, llvm::Value * lhs_val, llvm::Value * rhs_val);
    llvm::Value * createDiv(BType res_type, llvm::Value * lhs_val, llvm::Value * rhs_val);
    llvm::Value * createLessThan(BType lhs_type, BType rhs_type, llvm::Value * lhs_val, llvm::Value * rhs_val);
    llvm::Value * createGreaterThan(BType lhs_type, BType rhs_type, llvm::Value * lhs_val, llvm::Value * rhs_val);

    llvm::Value * createCast(llvm::Value * val, BType dest_type);
    llvm::Value * createIntToDoubleCast(llvm::Value * int_val);
    llvm::Value * createDoubleToIntCast(llvm::Value * double_val);
    llvm::Value * tryIntToDoubleCast(llvm::Value * maybe_int_val);
    llvm::Value * tryDoubleToIntCast(llvm::Value * maybe_double_val);
public:
    CodeGenerator();
    void printIR();

    void definePutChar();
    void generate(std::shared_ptr<BProgram> program);
    void optimize();
    void setTarget();
    void compile();

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