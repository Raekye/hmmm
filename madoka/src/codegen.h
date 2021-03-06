#ifndef __CODEGEN_H_
#define __CODEGEN_H_

#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/PassManager.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/CallingConv.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Analysis/Verifier.h>
#include <llvm/Assembly/PrintModulePass.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/JIT.h>
#include <llvm/Support/raw_ostream.h>
#include <map>
#include <stack>
#include "codescope.h"
#include "number.h"

class CodeGen {
public:
	llvm::Module* module;
	std::stack<llvm::BasicBlock*> blocks;
	CodeScope scope;
	CodeGen(llvm::Module*);
	
	void gen_code(NExpression* root);
	void run_code(); // GenericValue

	void push_block(llvm::BasicBlock*);
	void pop_block();
	llvm::BasicBlock* current_block();

	static llvm::Type* llvm_pointer_ty();
	static llvm::Value* llvm_implicit_cast_primitive_number(CodeGen*, NType*, llvm::Value*, NType*);
	static llvm::Value* llvm_cast_primitive_number(CodeGen*, NType*, llvm::Value*, NType*);

	llvm::IRBuilder<> builder;
};

#endif // __CODEGEN_H_
