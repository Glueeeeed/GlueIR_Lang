#include "AST/ast.h"
#include "codegen.h"

llvm::Type* CodeGenerator::getLLVMType(const std::string& typeName, bool isNullable) {
    llvm::Type* baseType = nullptr;
    if (typeName == "int") baseType = builder.getInt32Ty();
    else if (typeName == "double") baseType = builder.getDoubleTy();
    else if (typeName == "float") baseType = builder.getFloatTy();
    else if (typeName == "bool" || typeName == "boolean") baseType = builder.getInt1Ty();
    else if (typeName == "string") return builder.getPtrTy();
    else baseType = builder.getVoidTy();

    if (isNullable) {
        return llvm::StructType::get(context, { builder.getInt1Ty(), baseType });
    }
    return baseType;
}

llvm::Value* CodeGenerator::toBool(llvm::Value* val) {
    if (!val) return nullptr;
    llvm::Type* ty = val->getType();

    if (ty->isIntegerTy(1)) {
        return val;
    }
    if (ty->isStructTy()) {
        return builder.CreateExtractValue(val, 0, "has_val");
    }
    if (ty->isFloatingPointTy()) {
        return builder.CreateFCmpONE(val, llvm::ConstantFP::get(ty, 0.0), "tobool");
    }
    if (ty->isIntegerTy()) {
        return builder.CreateICmpNE(val, llvm::ConstantInt::get(ty, 0), "tobool");
    }
    if (ty->isPointerTy()) {
        return builder.CreateIsNotNull(val, "tobool");
    }
    return val;
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

void CodeGenerator::visitParameters(const ASTNode* paramsNode,llvm::Function *func) {
    size_t idx = 0;
    for (auto& arg : func->args()) {
         const auto& param = paramsNode->children[idx++];
        std::string pName = param->children[0]->value;
        std::string pType = param->children[1]->value;

        arg.setName(pName);

        llvm::Type* argLLVMType = getLLVMType(pType);
        llvm::AllocaInst* allocaInst = createEntryAlloca(func, builder, argLLVMType, pName);
        builder.CreateStore(&arg, allocaInst);

        namedValuesStack.back()[pName] = { allocaInst, getNodeType(pType) };
    }
}