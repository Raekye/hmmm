#ifndef __CODE_GEN_CONTEXT_H_
#define __CODE_GEN_CONTEXT_H_

#include <stack>
#include <list>
#include <llvm/Analysis/Passes.h>
#include <llvm/Analysis/Verifier.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/JIT.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/PassManager.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Transforms/Scalar.h>
#include "identifier_scope.h"
#include "ast_types_resolver.h"

class CodeGenContext {
public:
	llvm::Module* module;
	llvm::IRBuilder<> builder;
	llvm::LLVMContext& llvm_context;
	std::stack<llvm::BasicBlock*> blocks;
	IdentifierScope scope;
	ASTTypesResolver ast_types_resolver;

	CodeGenContext(llvm::LLVMContext& = llvm::getGlobalContext());
	~CodeGenContext();

	void push_block(llvm::BasicBlock*);
	void pop_block();
	void push_scope();
	void pop_scope();
	llvm::BasicBlock* current_block();

	llvm::Type* llvm_pointer_ty();
};

#endif /* __CODE_GEN_CONTEXT_H_ */
