#ifndef __CODEGEN_H_
#define __CODEGEN_H_

#include <stack>
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
//#include <llvm/ModuleProvider.h>
//#include <llvm/Target/TargetSelect.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/JIT.h>
#include <llvm/Support/raw_ostream.h>
#include <map>

class NBlock;

class CodeGenBlock {
public:
	llvm::BasicBlock* block;
	std::map<std::string, llvm::Value*> locals;
};

class CodeGenContext {
private:
	std::stack<CodeGenBlock*> blocks;
	llvm::Function* main_function;
public:
	llvm::Module* module;
	CodeGenContext();
	
	void generate_code(NExpression* root);
	void run_code(); // GenericValue
};

#endif // __CODEGEN_H_
