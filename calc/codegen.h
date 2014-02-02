#ifndef __CODEGEN_H_
#define __CODEGEN_H_

#include <stack>
#include <llvm/Module.h>
#include <llvm/Function.h>
#include <llvm/Type.h>
#include <llvm/DerivedTypes.h>
#include <llvm/LLVMContext.h>
#include <llvm/PassManager.h>
#include <llvm/Instructions.h>
#include <llvm/CallingConv.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Analysis/Verifier.h>
#include <llvm/Assembly/PrintModulePass.h>
#include <llvm/Support/IRBuilder.h>
//#include <llvm/ModuleProvider.h>
//#include <llvm/Target/TargetSelect.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/JIT.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm;

class NBlock;

class CodeGenBlock {
public:
	BasicBlock *block;
	std::map<std::string, Value*> locals;
};

class CodeGenContext {
private:
	std::stack<CodeGenBlock *> blocks;
	Function *mainFunction;

public:
	Module *module;
	CodeGenContext() { module = new Module("main", getGlobalContext()); }
	
	void generateCode(NExpression* root);
	void runCode(); // GenericValue
};

#endif // __CODEGEN_H_