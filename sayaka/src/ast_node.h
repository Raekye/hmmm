#ifndef __AST_NODE_H_
#define __AST_NODE_H_

#include <iostream>
#include <vector>
#include <tuple>
#include <llvm/IR/Value.h>
#include "ast_type.h"
#include "code_gen_context.h"

enum tagEBinaryOperationType {
	eMULTIPLY,
	eADD,
	eSUBTRACT,
	eDIVIDE,
	ePOW,
};

typedef enum tagEBinaryOperationType EBinaryOperationType;

class ASTNode {
public:
	ASTType* type;

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
	std::vector<ASTNode*> statements;

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

#endif /* __AST_NODE_H_ */
