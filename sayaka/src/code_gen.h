#ifndef __CODE_GEN_H_
#define __CODE_GEN_H_

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
#include "ast_node.h"

class CodeGen : public IASTNodeVisitor {
public:
	llvm::LLVMContext& context;
	llvm::Module* module;
	llvm::IRBuilder<> builder;
	std::stack<llvm::BasicBlock*> blocks;
	IdentifierScope scope;
	ASTTypesResolver ast_types_resolver;

	CodeGen(llvm::LLVMContext&, llvm::Module*);
	~CodeGen();

	void push_block(llvm::BasicBlock*);
	void pop_block();
	void push_scope();
	void pop_scope();
	llvm::BasicBlock* current_block();

	virtual void visit(ASTNodeIdentifier*) override;
	virtual void visit(ASTNodePrimitive*) override;
	virtual void visit(ASTNodeDeclaration*) override;
	virtual void visit(ASTNodeBlock*) override;
	virtual void visit(ASTNodeCast*) override;
	virtual void visit(ASTNodeAssignment*) override;
	virtual void visit(ASTNodeBinaryOperator*) override;
	virtual void visit(ASTNodeFunctionPrototype*) override;
	virtual void visit(ASTNodeFunction*) override;
	virtual void visit(ASTNodeFunctionCall*) override;
	virtual void visit(ASTNodeIfElse*) override;
};

#endif /* __CODE_GEN_H_ */

