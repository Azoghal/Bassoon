#include "ast.hxx"

namespace bassoon
{
    
llvm::Value *BoolExprAST::codegen(){
    return nullptr;
}

llvm::Value *IntExprAST::codegen(){
    return nullptr;
}

llvm::Value *DoubleExprAST::codegen(){
    return nullptr;
}

llvm::Value *UnaryExprAST::codegen(){
    return nullptr;
}

llvm::Value *BinaryExprAST::codegen(){
    return nullptr;
}

llvm::Value *CallExprAST::codegen(){
    return nullptr;
}

llvm::Value *VariableExprAST::codegen(){
    return nullptr;
}

} // namespace bassoon
