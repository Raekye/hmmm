#ifndef __AST_NODE_H_
#define __AST_NODE_H_

#include <iostream>
#include <vector>
#include <tuple>
#include <llvm/IR/Value.h>
#include "ast_type.h"
#include "code_gen_context.h"

enum tagEBinaryOperationType {
	eADD = 1,
	eSUBTRACT = 3,
	eMULTIPLY = 5,
	eDIVIDE = 7,
	ePOW = 9,
	eEQ = 0,
	eNEQ = 2,
	eLT = 4,
	eGT = 6,
	eLEQ = 8,
	eGEQ = 10,
};

typedef enum tagEBinaryOperationType EBinaryOperationType;

class IASTNodeVisitor;

class ASTNode {
public:
	ASTType* type;

	void accept(IASTNodeVisitor*);

	virtual ~ASTNode() = 0;
	virtual llvm::Value* gen_code(CodeGenContext*) = 0;
	virtual ASTNode* pass_types(CodeGenContext*, ASTType*) = 0;
};

class ASTNodeIdentifier : public ASTNode {
public:
	std::string name;

	ASTNodeIdentifier(std::string);

	virtual ~ASTNodeIdentifier();
	virtual llvm::Value* gen_code(CodeGenContext*) override;
	virtual ASTNodeIdentifier* pass_types(CodeGenContext*, ASTType*) override;
};

class ASTNodePrimitive : public ASTNode {
public:
	std::string str;

	ASTNodePrimitive(std::string);

	virtual ~ASTNodePrimitive();
	virtual llvm::Value* gen_code(CodeGenContext*) override;
	virtual ASTNodePrimitive* pass_types(CodeGenContext*, ASTType*) override;
};

class ASTNodeDeclaration : public ASTNode {
public:
	std::string type_name;
	std::string var_name;

	ASTNodeDeclaration(std::string, std::string);

	virtual ~ASTNodeDeclaration();
	virtual llvm::Value* gen_code(CodeGenContext*) override;
	virtual ASTNodeDeclaration* pass_types(CodeGenContext*, ASTType*) override;
};

class ASTNodeBlock : public ASTNode {
public:
	std::vector<ASTNode*> statements; // TODO: make vector* so consistent

	ASTNodeBlock();
	void push(ASTNode*);

	virtual ~ASTNodeBlock();
	virtual llvm::Value* gen_code(CodeGenContext*) override;
	virtual ASTNodeBlock* pass_types(CodeGenContext*, ASTType*) override;
};

class ASTNodeCast : public ASTNode {
public:
	std::string target_type;
	ASTNode* val;

	ASTNodeCast(std::string, ASTNode*);

	virtual ~ASTNodeCast();
	virtual llvm::Value* gen_code(CodeGenContext*) override;
	virtual ASTNodeCast* pass_types(CodeGenContext*, ASTType*) override;
};

class ASTNodeAssignment : public ASTNode {
public:
	ASTNodeIdentifier* lhs;
	ASTNode* rhs;

	ASTNodeAssignment(ASTNodeIdentifier*, ASTNode*);

	virtual ~ASTNodeAssignment();
	virtual llvm::Value* gen_code(CodeGenContext*) override;
	virtual ASTNodeAssignment* pass_types(CodeGenContext*, ASTType*) override;
};

class ASTNodeBinaryOperator : public ASTNode {
public:
	EBinaryOperationType op;
	ASTNode* lhs;
	ASTNode* rhs;

	ASTNodeBinaryOperator(EBinaryOperationType, ASTNode*, ASTNode*);

	virtual ~ASTNodeBinaryOperator();
	virtual llvm::Value* gen_code(CodeGenContext*) override;
	virtual ASTNodeBinaryOperator* pass_types(CodeGenContext*, ASTType*) override;
};

class ASTNodeFunctionPrototype : public ASTNode {
public:
	std::string return_type;
	std::string function_name;
	std::vector<ASTNodeDeclaration*>* args;

	ASTNodeFunctionPrototype(std::string, std::string, std::vector<ASTNodeDeclaration*>*);

	virtual ~ASTNodeFunctionPrototype();
	virtual llvm::Value* gen_code(CodeGenContext*) override;
	virtual ASTNodeFunctionPrototype* pass_types(CodeGenContext*, ASTType*) override;
};

class ASTNodeFunction : public ASTNode {
public:
	ASTNodeFunctionPrototype* prototype;
	ASTNodeBlock* body;

	ASTNodeFunction(ASTNodeFunctionPrototype*, ASTNodeBlock*);

	virtual ~ASTNodeFunction();
	virtual llvm::Value* gen_code(CodeGenContext*) override;
	virtual ASTNodeFunction* pass_types(CodeGenContext*, ASTType*) override;
};

class ASTNodeFunctionCall : public ASTNode {
public:
	std::string function_name;
	std::vector<ASTNode*>* args;

	ASTNodeFunctionCall(std::string, std::vector<ASTNode*>*);

	virtual ~ASTNodeFunctionCall();
	virtual llvm::Value* gen_code(CodeGenContext*) override;
	virtual ASTNodeFunctionCall* pass_types(CodeGenContext*, ASTType*) override;
};

class ASTNodeIfElse : public ASTNode {
public:
	ASTNode* cond;
	ASTNode* if_true;
	ASTNode* if_false;

	ASTNodeIfElse(ASTNode* cond, ASTNode* if_true, ASTNode* if_false);

	virtual ~ASTNodeIfElse();
	virtual llvm::Value* gen_code(CodeGenContext*) override;
	virtual ASTNodeIfElse* pass_types(CodeGenContext*, ASTType*) override;
};

class IASTNodeVisitor {
public:
	void visit(ASTNode*);
	virtual void visit(ASTNodeIdentifier*) = 0;
	virtual void visit(ASTNodePrimitive*) = 0;
	virtual void visit(ASTNodeDeclaration*) = 0;
	virtual void visit(ASTNodeBlock*) = 0;
	virtual void visit(ASTNodeCast*) = 0;
	virtual void visit(ASTNodeAssignment*) = 0;
	virtual void visit(ASTNodeBinaryOperator*) = 0;
	virtual void visit(ASTNodeFunctionPrototype*) = 0;
	virtual void visit(ASTNodeFunction*) = 0;
	virtual void visit(ASTNodeFunctionCall*) = 0;
	virtual void visit(ASTNodeIfElse*) = 0;
};

#endif /* __AST_NODE_H_ */
