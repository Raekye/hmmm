#ifndef MIDORI_CODEGEN_H_INCLUDED
#define MIDORI_CODEGEN_H_INCLUDED

#include "lang.h"
#include "types.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Type.h"
#include <list>
#include <map>
#include <system_error>

class CodeGen : public ILangASTVisitor {
public:
	CodeGen();
	std::error_code dump(std::string path);
	virtual void visit(LangASTIdent*) override;
	virtual void visit(LangASTDecl*) override;
	virtual void visit(LangASTBinOp*) override;
	virtual void visit(LangASTInt*) override;
	virtual void visit(LangASTDouble*) override;
	virtual void visit(LangASTIf*) override;
	virtual void visit(LangASTWhile*) override;
	virtual void visit(LangASTPrototype*) override;
	virtual void visit(LangASTFunction*) override;
	virtual void visit(LangASTCall*) override;

private:
	llvm::LLVMContext context;
	llvm::IRBuilder<> builder;
	llvm::Module module;
	std::list<std::map<std::string, llvm::Value*>> frames;
	llvm::Value* ret;
	llvm::Function* ret_function;
	TypeManager type_manager;

	llvm::Value* named_value(std::string);
	llvm::Function* get_function(std::string);
};

#endif /* MIDORI_CODEGEN_H_INCLUDED */
