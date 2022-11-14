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

llvm::Value *BlockStatementAST::codegen(){
    return nullptr;
}

llvm::Value *IfStatementAST::codegen(){
    return nullptr;
}

llvm::Value *CallStatementAST::codegen(){
    return nullptr;
}

llvm::Value *ReturnStatementAST::codegen(){
    return nullptr;
}

llvm::Value *AssignStatementAST::codegen(){
    return nullptr;
}

llvm::Value *InitStatementAST::codegen(){
    return nullptr;
}

llvm::Value *ForStatementAST::codegen(){
    return nullptr;
}

llvm::Value *WhileStatementAST::codegen(){
    return nullptr;
}

} // namespace bassoon
