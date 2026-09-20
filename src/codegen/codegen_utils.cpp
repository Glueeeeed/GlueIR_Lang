#include "AST/ast.h"
#include "codegen.h"

llvm::Type* CodeGenerator::getLLVMType(const std::string& typeName) {
     if (typeName == "int") return builder.getInt32Ty();
     if (typeName == "string") return builder.getPtrTy();
     if (typeName == "bool" || typeName == "boolean") return builder.getInt1Ty();
     if (typeName == "double") return builder.getDoubleTy();
     if (typeName == "float") return builder.getFloatTy();
         return builder.getVoidTy();
}

NodeType CodeGenerator::getNodeType(const std::string& typeName) {
     if (typeName == "int") return NodeType::NUMBER;
     if (typeName == "double") return NodeType::NUMBER_DOUBLE;
     if (typeName == "float") return NodeType::NUMBER_FLOAT;
     if (typeName == "bool" || typeName == "boolean") return NodeType::BOOLEAN;
     if (typeName == "string") return NodeType::STRING;
        return NodeType::BOND;
}

void CodeGenerator::generateSrandSeed() {
    auto timeFunc = module->getOrInsertFunction("time", llvm::FunctionType::get(builder.getInt64Ty(), { builder.getPtrTy() }, false));
    llvm::Value* nullPtr = llvm::ConstantPointerNull::get(builder.getPtrTy());
    llvm::Value* timeVal = builder.CreateCall(timeFunc, { nullPtr });
    llvm::Value* seedVal = builder.CreateTrunc(timeVal, builder.getInt32Ty());
    auto srandFunc = module->getOrInsertFunction("srand", llvm::FunctionType::get(builder.getVoidTy(), { builder.getInt32Ty() }, false));
    builder.CreateCall(srandFunc, { seedVal });
}