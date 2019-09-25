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
	void process(LangAST*);
	std::error_code dump(std::string);
	void run();
	virtual void visit(LangASTBasicType*) override;
	virtual void visit(LangASTPointerType*) override;
	virtual void visit(LangASTArrayType*) override;
	virtual void visit(LangASTBlock*) override;
	virtual void visit(LangASTLIdent*) override;
	virtual void visit(LangASTRIdent*) override;
	virtual void visit(LangASTAssignment*) override;
	virtual void visit(LangASTDecl*) override;
	virtual void visit(LangASTUnOp*) override;
	virtual void visit(LangASTBinOp*) override;
	virtual void visit(LangASTInt*) override;
	virtual void visit(LangASTDouble*) override;
	virtual void visit(LangASTIf*) override;
	virtual void visit(LangASTWhile*) override;
	virtual void visit(LangASTPrototype*) override;
	virtual void visit(LangASTFunction*) override;
	virtual void visit(LangASTReturn*) override;
	virtual void visit(LangASTCall*) override;
	virtual void visit(LangASTClassDef*) override;

private:
	struct Variable {
		Type* type;
		llvm::Value* value;

		Variable(Type* t, llvm::Value* v) : type(t), value(v) {
			return;
		}
	};
	llvm::LLVMContext context;
	llvm::IRBuilder<> builder;
	std::unique_ptr<llvm::Module> module;
	std::list<std::map<std::string, Variable>> frames;
	llvm::Value* ret;
	TypeManager type_manager;

	void push_scope();
	void pop_scope();
	Variable named_value(std::string);
	void set_named_value(std::string, Variable);
	llvm::Function* get_function(std::string);
};

#endif /* MIDORI_CODEGEN_H_INCLUDED */
