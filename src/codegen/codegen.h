#ifndef GLUESCRIPTCOMPILER_CODEGEN_H
#define GLUESCRIPTCOMPILER_CODEGEN_H

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "../AST/ast.h"
#include <iostream>
#include <map>
#include <sstream>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>

struct namedValuesStruct {
    llvm::AllocaInst* pointer;
    NodeType nodeType;
};


class CodeGenerator {

    std::vector<std::unordered_map<std::string, namedValuesStruct>> namedValuesStack;

    llvm::LLVMContext &context;
    std::unique_ptr<llvm::Module> module;
    llvm::IRBuilder<> builder;
    llvm::BasicBlock* currentExitBlock;
    llvm::Function* printf = nullptr;
    llvm::Function* currentFunction = nullptr;
    void enterScope();
    void exitScope();
    namedValuesStruct* lookupNamedValue(const std::string& name);
    void visit(const ASTNode* node);
    void visitDeclaration(const ASTNode* node);
    void visitAssignment(const ASTNode* node);
    void visitIfStatement(const ASTNode* node);
    void visitWhileStatement(const ASTNode* node);
    llvm::Value* visitFunction(const ASTNode* node);
    llvm::Value* visitExpression(const ASTNode* node);

    // void newHeapString(std::string str, llvm::Value *type);
    // llvm::Value* createHeapString(std::string str);

    void expect(std::string msg);
    static llvm::AllocaInst* createEntryAlloca(llvm::Function* F, llvm::IRBuilder<> &B, llvm::Type* ty, const std::string &name) {
        llvm::IRBuilder<> tmp(&F->getEntryBlock(), F->getEntryBlock().begin());
        return tmp.CreateAlloca(ty, nullptr, name);
    }


    // utils  

    llvm::Type* getLLVMType(const std::string& typeName);
    NodeType getNodeType(const std::string& typeName);




public:



    explicit CodeGenerator(llvm::LLVMContext &ctx)
    : context(ctx), builder(context) {}
    void generateCode(const ASTNode* node);
    void generate(const ASTNode* node);
    void save();
    void run();
};

#endif //GLUESCRIPTCOMPILER_CODEGEN_H