#include "ast.hxx"

namespace bassoon
{
    
llvm::Value *BoolExprAST::codegen(){
    return nullptr;
}

llvm::Value *UnaryExprAST::codegen(){
    return nullptr;
}

llvm::Value *BinaryExprAST::codegen(){
    return nullptr;
}

} // namespace bassoon
