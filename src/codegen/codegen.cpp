#include "codegen.h"

#include <sstream>
#include "../parser/parser.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/TargetParser/Host.h"
#include "llvm/TargetParser/Triple.h"

void CodeGenerator::generateCode(const ASTNode *node) {
    module = std::make_unique<llvm::Module>("glue_auto", context);
    module->setTargetTriple(llvm::Triple(llvm::sys::getDefaultTargetTriple()));

    generate(node);
    
    save();
}

void CodeGenerator::enterScope() {
    namedValuesStack.emplace_back();
}

void CodeGenerator::exitScope() {
    if (!namedValuesStack.empty()) {
        namedValuesStack.pop_back();
    }
}

namedValuesStruct* CodeGenerator::lookupNamedValue(const std::string& name) {
    for (auto it = namedValuesStack.rbegin(); it != namedValuesStack.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            return &(found->second);
        }
    }
    return nullptr;
}


void CodeGenerator::generate(const ASTNode *node) {
    if (!node) return;

    switch (node->type) {
        case NodeType::PROGRAM:
        case NodeType::BLOCK:
            enterScope();
            for (const auto& child : node->children) {
                generate(child.get());
            }
            exitScope();
            break;
        case NodeType::FUNCTION_DECLARATION: {
            std::string funcName = node->value;
            funcName = (funcName == "Main") ? "main" : funcName;


            std::string returnTypeName = (!node->children.empty() && node->children[0]->type == NodeType::TYPE)
                                         ? node->children[0]->value
                                         : "void";



            llvm::Type* retType = getLLVMType(returnTypeName);

            std::vector<llvm::Type*> paramTypes;
            const ASTNode* paramsNode = (node->children.size() > 2) ? node->children[1].get() : nullptr;

            if (paramsNode) {
                for (const auto& param : paramsNode->children) {
                    std::string pType = param->children[1]->value;
                    paramTypes.push_back(getLLVMType(pType));
                }
            }

            llvm::FunctionType *funcType = llvm::FunctionType::get(retType, paramTypes, false);

            llvm::Function *func = llvm::Function::Create(
                funcType, llvm::Function::ExternalLinkage, funcName, module.get());

            currentFunction = func;

            llvm::BasicBlock *entry = llvm::BasicBlock::Create(context, "entry", func);
            builder.SetInsertPoint(entry);


            // generate random seed for srand

            if (funcName == "main") {
                generateSrandSeed();
            }


            enterScope();


            if (paramsNode) {
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

            if (funcName == "main") {
                this->currentExitBlock = llvm::BasicBlock::Create(context, "exit", func);
            }
            llvm::BasicBlock* ExitBB = this->currentExitBlock;

            const ASTNode* bodyNode = node->children.back().get();
            if (bodyNode && bodyNode->type == NodeType::BLOCK) {
                for (const auto& child : bodyNode->children) {
                    generate(child.get());
                }
            }

            exitScope();

            if (funcName == "main") {


                if (auto *existingTerm = builder.GetInsertBlock()->getTerminator()) {
                    existingTerm->eraseFromParent();
                }

                builder.CreateBr(ExitBB);

                builder.SetInsertPoint(ExitBB);

                auto exitInfo = module->getOrInsertFunction("printf", llvm::FunctionType::get(builder.getInt32Ty(), { builder.getPtrTy() }, true));
                builder.CreateCall(exitInfo, builder.CreateGlobalString("Program completed successfully. Press Enter to exit.\n"));

                auto *getcharType = llvm::FunctionType::get(builder.getInt32Ty(), false);
                auto getcharFunc = module->getOrInsertFunction("getchar", getcharType);
                builder.CreateCall(getcharFunc);

                builder.CreateRet(builder.getInt32(0));
            } else {
                if (!builder.GetInsertBlock()->getTerminator()) {
                    if (currentFunction->getReturnType()->isVoidTy()) {
                        builder.CreateRetVoid();
                    } else if (currentFunction->getReturnType()->isIntegerTy(1)) {
                        builder.CreateRet(builder.getInt1(false));
                    } else if (currentFunction->getReturnType()->isPointerTy()) {
                        builder.CreateRet(llvm::ConstantPointerNull::get(builder.getPtrTy()));
                    } else if (currentFunction->getReturnType()->isDoubleTy()) {
                        builder.CreateRet(llvm::ConstantFP::get(builder.getDoubleTy(), 0.0));
                    } else if (currentFunction->getReturnType()->isFloatTy()) {
                        builder.CreateRet(llvm::ConstantFP::get(builder.getFloatTy(), 0.0f));
                    } else {
                        builder.CreateRet(builder.getInt32(0));
                    }
                }
            }
            break;
        }
        case NodeType::RETURN_STATEMENT:
            if (currentFunction->getName() == "main") {
                builder.CreateBr(this->currentExitBlock);
            } else if (!node->children.empty()) {
                llvm::Value* retVal = visitExpression(node->children[0].get());
                builder.CreateRet(retVal);
            } else {
                builder.CreateRetVoid();
            }
            break;
        case NodeType::DECLARATION:
            visitDeclaration(node);
            break;
        case NodeType::ASSIGNMENT:
            visitAssignment(node);
            break;
        case NodeType::FUNCTION_CALL:
            visitFunction(node);
            break;
        case NodeType::IF_STATEMENT:
            visitIfStatement(node);
            break;
        case NodeType::WHILE_STATEMENT:
            visitWhileStatement(node);
            break;
        default:
            break;
    }
}

void CodeGenerator::visitIfStatement(const ASTNode* node) {
    llvm::Value* condVal = visitExpression(node->children[0].get());
    if (!condVal) return;

    if (condVal->getType()->isFloatingPointTy()) {
        condVal = builder.CreateFCmpONE(condVal, llvm::ConstantFP::get(condVal->getType(), 0.0), "ifcond");
    } else if (condVal->getType()->isIntegerTy() && !condVal->getType()->isIntegerTy(1)) {
        condVal = builder.CreateIsNotNull(condVal, "ifcond");
    } else if (condVal->getType()->isPointerTy()) {
        condVal = builder.CreateIsNotNull(condVal, "ifcond");
    }

    llvm::Function* func = builder.GetInsertBlock()->getParent();
    llvm::BasicBlock* thenBB = llvm::BasicBlock::Create(context, "then", func);
    llvm::BasicBlock* elseBB = llvm::BasicBlock::Create(context, "else");
    llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(context, "ifcont");

    bool hasElse = node->children.size() > 2;

    builder.CreateCondBr(condVal, thenBB, hasElse ? elseBB : mergeBB);

    builder.SetInsertPoint(thenBB);
    generate(node->children[1].get());
    if (!builder.GetInsertBlock()->getTerminator()) {
        builder.CreateBr(mergeBB);
    }

    if (hasElse) {
        func->insert(func->end(), elseBB);
        builder.SetInsertPoint(elseBB);
        generate(node->children[2].get());
        if (!builder.GetInsertBlock()->getTerminator()) {
            builder.CreateBr(mergeBB);
        }
    }

    func->insert(func->end(), mergeBB);
    builder.SetInsertPoint(mergeBB);
}


void CodeGenerator::visitWhileStatement(const ASTNode* node) {
    llvm::Function* func = builder.GetInsertBlock()->getParent();

    llvm::BasicBlock* condBB = llvm::BasicBlock::Create(context, "while.cond", func);
    llvm::BasicBlock* bodyBB = llvm::BasicBlock::Create(context, "while.body");
    llvm::BasicBlock* endBB  = llvm::BasicBlock::Create(context, "while.end");

    builder.CreateBr(condBB);

    builder.SetInsertPoint(condBB);
    llvm::Value* condVal = visitExpression(node->children[0].get());
    if (!condVal) return;


    if (condVal->getType()->isFloatingPointTy()) {
        condVal = builder.CreateFCmpONE(condVal, llvm::ConstantFP::get(condVal->getType(), 0.0), "whilecond");
    } else if (condVal->getType()->isIntegerTy() && !condVal->getType()->isIntegerTy(1)) {
        condVal = builder.CreateIsNotNull(condVal, "whilecond");
    }

    builder.CreateCondBr(condVal, bodyBB, endBB);

    func->insert(func->end(), bodyBB);
    builder.SetInsertPoint(bodyBB);
    generate(node->children[1].get());

    if (!builder.GetInsertBlock()->getTerminator()) {
        builder.CreateBr(condBB);
    }

    func->insert(func->end(), endBB);
    builder.SetInsertPoint(endBB);
}



void CodeGenerator::visitDeclaration(const ASTNode *node) {
    const ASTNode* idNode   = node->children[0].get();
    const ASTNode* typeNode = node->children[1].get();
    const ASTNode* valNode  = node->children[2].get();

    std::string varName = idNode->value;
    llvm::Value* val = visitExpression(valNode);

    llvm::Type* ty = nullptr;
    NodeType nt = NodeType::BOND;
    if (typeNode->value == "int") {
        ty = builder.getInt32Ty();
        nt = NodeType::NUMBER;
        if (val->getType()->isFloatingPointTy()) {
            val = builder.CreateFPToSI(val, ty, "castit");
        }
    } else if (typeNode->value == "double") {
        ty = builder.getDoubleTy();
        nt = NodeType::NUMBER_DOUBLE;
        if (!val->getType()->isDoubleTy()) {
            if (val->getType()->isFloatingPointTy()) {
                val = builder.CreateFPExt(val, ty, "castd");
            } else {
                val = builder.CreateSIToFP(val, ty, "castd");
            }
        }
    } else if (typeNode->value == "float") {
        ty = builder.getFloatTy();
        nt = NodeType::NUMBER_FLOAT;
        if (!val->getType()->isFloatTy()) {
            if (val->getType()->isFloatingPointTy()) {
                val = builder.CreateFPTrunc(val, ty, "castf");
            } else {
                val = builder.CreateSIToFP(val, ty, "castf");
            }
        }
    } else if (typeNode->value == "string") {
        ty = builder.getPtrTy();
        nt = NodeType::STRING;
    } else if (typeNode->value == "bool" || typeNode->value == "boolean") {
        ty = builder.getInt1Ty();
        nt = NodeType::BOOLEAN;
        if (val->getType()->isIntegerTy() && !val->getType()->isIntegerTy(1)) {
            val = builder.CreateIsNotNull(val, "boolcast");
        }
    } else {
        expect("Compile Error: unknown type '" + typeNode->value + "'");
        return;
    }

    auto* allocaInst = createEntryAlloca(currentFunction, builder, ty, varName);
    builder.CreateStore(val, allocaInst);
    

    if (!namedValuesStack.empty()) {
        namedValuesStack.back()[varName] = { allocaInst, nt };
    }
}
void CodeGenerator::visitAssignment(const ASTNode *node) {
    const ASTNode* idNode   = node->children[0].get();
    const ASTNode* valNode  = node->children[1].get();

    std::string varName = idNode->value;
    namedValuesStruct* var = lookupNamedValue(varName);
    if (!var) {
        expect("Compile Error: variable '" + varName + "' not found");
        return;
    }

    llvm::Value* val = visitExpression(valNode);

    if (var->nodeType == NodeType::NUMBER) {
        if (val->getType()->isFloatingPointTy()) {
            val = builder.CreateFPToSI(val, builder.getInt32Ty(), "castit");
        }
    } else if (var->nodeType == NodeType::NUMBER_DOUBLE) {
        if (!val->getType()->isDoubleTy()) {
            if (val->getType()->isFloatingPointTy()) {
                val = builder.CreateFPExt(val, builder.getDoubleTy(), "castd");
            } else {
                val = builder.CreateSIToFP(val, builder.getDoubleTy(), "castd");
            }
        }
    } else if (var->nodeType == NodeType::NUMBER_FLOAT) {
        if (!val->getType()->isFloatTy()) {
            if (val->getType()->isFloatingPointTy()) {
                val = builder.CreateFPTrunc(val, builder.getFloatTy(), "castf");
            } else {
                val = builder.CreateSIToFP(val, builder.getFloatTy(), "castf");
            }
        }
    } else if (var->nodeType == NodeType::BOOLEAN) {
        if (!val->getType()->isIntegerTy(1)) {
            val = builder.CreateIsNotNull(val, "boolcast");
        }
    }

    builder.CreateStore(val, var->pointer);
}

llvm::Value* CodeGenerator::visitFunction(const ASTNode* node) {
    std::string functionName = node->value;


    if (functionName == "shin") {
        auto mallocFunc = module->getOrInsertFunction("malloc", llvm::FunctionType::get(builder.getPtrTy(), { builder.getInt64Ty() }, false));
        llvm::Value* buffer = builder.CreateCall(mallocFunc, { builder.getInt64(1024) });

        auto scanfFunc = module->getOrInsertFunction("scanf", llvm::FunctionType::get(builder.getInt32Ty(), { builder.getPtrTy() }, true));
        llvm::Value* fmt = builder.CreateGlobalString("%1023s");
        builder.CreateCall(scanfFunc, { fmt, buffer });

        return buffer;
    }

    if (functionName == "toInt") {
        llvm::Value* strVal = visitExpression(node->children[0].get());
        auto atoiFunc = module->getOrInsertFunction("atoi", llvm::FunctionType::get(builder.getInt32Ty(), { builder.getPtrTy() }, false));
        return builder.CreateCall(atoiFunc, { strVal });
    }

    if (functionName == "toDouble") {
        llvm::Value* strVal = visitExpression(node->children[0].get());
        auto atofFunc = module->getOrInsertFunction("atof", llvm::FunctionType::get(builder.getDoubleTy(), { builder.getPtrTy() }, false));
        return builder.CreateCall(atofFunc, { strVal });
    }

    if (functionName == "toFloat") {
        llvm::Value* strVal = visitExpression(node->children[0].get());
        auto atofFunc = module->getOrInsertFunction("atof", llvm::FunctionType::get(builder.getDoubleTy(), { builder.getPtrTy() }, false));
        llvm::Value* dVal = builder.CreateCall(atofFunc, { strVal });
        return builder.CreateFPTrunc(dVal, builder.getFloatTy());
    }

    if (functionName == "random") {
        llvm::Value* minVal = visitExpression(node->children[0].get());
        llvm::Value* maxVal = visitExpression(node->children[1].get());

        auto randFunc = module->getOrInsertFunction("rand", llvm::FunctionType::get(builder.getInt32Ty(), false));
        llvm::Value* r = builder.CreateCall(randFunc);

        // range = (max - min) + 1
        llvm::Value* diff = builder.CreateSub(maxVal, minVal);
        llvm::Value* range = builder.CreateAdd(diff, builder.getInt32(1));

        // result = min + (rand() % range)
        llvm::Value* mod = builder.CreateSRem(r, range);
        return builder.CreateAdd(minVal, mod);
    }

    if (functionName == "shout") {
        auto printfFunc = module->getOrInsertFunction("printf", llvm::FunctionType::get(builder.getInt32Ty(), { builder.getPtrTy() }, true));
        std::string format;
        std::vector<llvm::Value*> args;

        for (size_t i = 0; i < node->children.size(); ++i) {
            const ASTNode* arg = node->children[i].get();
            if (!arg) continue;

            llvm::Value* val = visitExpression(arg);
            if (!val) continue;

            llvm::Type* type = val->getType();
            if (type->isIntegerTy(1)) {
                format += "%d";
                args.push_back(builder.CreateZExt(val, builder.getInt32Ty()));
            } else if (type->isIntegerTy()) {
                format += "%d";
                args.push_back(val);
            } else if (type->isDoubleTy()) {
                format += "%g";
                args.push_back(val);
            } else if (type->isFloatTy()) {
                format += "%g";
                args.push_back(builder.CreateFPExt(val, builder.getDoubleTy()));
            } else if (type->isPointerTy()) {
                format += "%s";
                args.push_back(val);
            }
        }

        format += "\n";
        llvm::Value* fmt = builder.CreateGlobalString(format);
        args.insert(args.begin(), fmt);
        return builder.CreateCall(printfFunc, args);

    }

    llvm::Function* callee = module->getFunction(functionName);
    if (!callee) {
        expect("Compile Error: function '" + functionName + "' not found");
    }

    std::vector<llvm::Value*> args;
    for (const auto& child : node->children) {
        args.push_back(visitExpression(child.get()));
    }

    return builder.CreateCall(callee, args);
}

llvm::Value* CodeGenerator::visitExpression(const ASTNode *node) {
    if (!node) return nullptr;

    switch (node->type) {
        case NodeType::NUMBER:
            return builder.getInt32(std::stoi(node->value));
        case NodeType::NUMBER_DOUBLE:
            return llvm::ConstantFP::get(context, llvm::APFloat(std::stod(node->value)));
        case NodeType::NUMBER_FLOAT:
            return llvm::ConstantFP::get(context, llvm::APFloat(std::stof(node->value)));
        case NodeType::STRING:
            return builder.CreateGlobalString(node->value);
        case NodeType::IDENTIFIER: {
            namedValuesStruct* var = lookupNamedValue(node->value);
            if (var) {
                llvm::Type* ty;
                if (var->nodeType == NodeType::NUMBER) {
                    ty = builder.getInt32Ty();
                } else if (var->nodeType == NodeType::NUMBER_DOUBLE) {
                    ty = builder.getDoubleTy();
                } else if (var->nodeType == NodeType::NUMBER_FLOAT) {
                    ty = builder.getFloatTy();
                } else if (var->nodeType == NodeType::BOOLEAN) {
                    ty = builder.getInt1Ty();
                } else {
                    ty = builder.getPtrTy();
                }
                return builder.CreateLoad(ty, var->pointer, node->value);
            }
            expect("Compile Error: variable '" + node->value + "' not found");
        }
        case NodeType::BINARY_OPERATION: {
            llvm::Value* L = visitExpression(node->children[0].get());
            llvm::Value* R = visitExpression(node->children[1].get());
            if (!L || !R) return nullptr;

            bool isLFP = L->getType()->isFloatingPointTy();
            bool isRFP = R->getType()->isFloatingPointTy();

            if (isLFP && !isRFP) {
                R = builder.CreateSIToFP(R, L->getType(), "promoter");
            } else if (!isLFP && isRFP) {
                L = builder.CreateSIToFP(L, R->getType(), "promotel");
            } else if (isLFP && isRFP) {
                if (L->getType()->isDoubleTy() && R->getType()->isFloatTy()) {
                    R = builder.CreateFPExt(R, builder.getDoubleTy(), "promoter");
                } else if (L->getType()->isFloatTy() && R->getType()->isDoubleTy()) {
                    L = builder.CreateFPExt(L, builder.getDoubleTy(), "promotel");
                }
            }

            bool isFP = L->getType()->isFloatingPointTy() || R->getType()->isFloatingPointTy();

            if (node->value == "+") {
                return isFP ? builder.CreateFAdd(L, R, "addtmp") : builder.CreateAdd(L, R, "addtmp");
            } else if (node->value == "-") {
                return isFP ? builder.CreateFSub(L, R, "subtmp") : builder.CreateSub(L, R, "subtmp");
            } else if (node->value == "*") {
                return isFP ? builder.CreateFMul(L, R, "multmp") : builder.CreateMul(L, R, "multmp");
            } else if (node->value == "/") {
                return isFP ? builder.CreateFDiv(L, R, "divtmp") : builder.CreateSDiv(L, R, "divtmp");
            } else if (node->value == "==") {
                return isFP ? builder.CreateFCmpOEQ(L, R, "cmptmp") : builder.CreateICmpEQ(L, R, "cmptmp");
            } else if (node->value == "!=") {
                return isFP ? builder.CreateFCmpONE(L, R, "cmptmp") : builder.CreateICmpNE(L, R, "cmptmp");
            } else if (node->value == "<") {
                return isFP ? builder.CreateFCmpOLT(L, R, "cmptmp") : builder.CreateICmpSLT(L, R, "cmptmp");
            } else if (node->value == ">") {
                return isFP ? builder.CreateFCmpOGT(L, R, "cmptmp") : builder.CreateICmpSGT(L, R, "cmptmp");
            } else if (node->value == "<=") {
                return isFP ? builder.CreateFCmpOLE(L, R, "cmptmp") : builder.CreateICmpSLE(L, R, "cmptmp");
            } else if (node->value == ">=") {
                return isFP ? builder.CreateFCmpOGE(L, R, "cmptmp") : builder.CreateICmpSGE(L, R, "cmptmp");
            } if (node->value == "and") {
                return builder.CreateAnd(L, R, "andtmp");
            } else if (node->value == "or") {
                return builder.CreateOr(L, R, "ortmp");
            }
        }
        case NodeType::BOOLEAN:
            return builder.getInt1(node->value == "true");
        case NodeType::FUNCTION_CALL: {
            return visitFunction(node);
        }
        default:
            return nullptr;
    }
}


// llvm::Value *CodeGenerator::createHeapString(std::string str) {
//     auto len = str.length();
//     auto* value = builder.getInt64Ty();
//     llvm::Constant* size = llvm::ConstantInt::get(value, len + 1);
//
//     llvm::DataLayout dataLayout = builder.GetInsertBlock()->getModule()->getDataLayout();
//     llvm::Type *intPtrTy = builder.getIntPtrTy(dataLayout);
//
//     auto* ptr = builder.CreateMalloc(intPtrTy, value, size, nullptr);
//     llvm::Constant* src = builder.CreateGlobalString(str + '\0');
//     builder.CreateMemCpy(ptr, std::nullopt, src, std::nullopt, size);
//     return ptr;
// }


// void CodeGenerator::newHeapString(std::string str, llvm::Value *type) {
//     llvm::Value* oldPtr = builder.CreateLoad(builder.getPtrTy(), type);
//     builder.CreateFree(oldPtr);
//     llvm::Value* value = createHeapString(str);
//     builder.CreateStore(value, type);
// }

void CodeGenerator::save() {
    std::error_code EC;
    llvm::raw_fd_ostream out("gir.ll", EC);
    module->print(out, nullptr);
    run();
}

void CodeGenerator::run() {
#if defined(_WIN32) || defined(_WIN64)

    // temporary solution

    system("clang -O2 gir.ll -o glue_program.exe -llegacy_stdio_definitions");
    system("glue_program.exe");
    system("del gir.ll");
#else
    system("clang -O2 gir.ll -o glue_program && ./glue_program");
    system("rm gir.ll");
#endif
    // system("clang gir.ll -o glue_bench"); // BENCHMARKS

}

void CodeGenerator::expect(std::string msg) {
    std::stringstream stream;
    stream << msg << std::endl;
    throw ParseError(stream.str());
}
