#ifndef __AST_NODE_H_
#define __AST_NODE_H_

#include <iostream>
#include <llvm/IR/Value.h>
#include "ast_type.h"
#include "code_gen_context.h"

union tagUNumberValue {
	int8_t b;
	uint8_t ub;
	int16_t s;
	uint16_t us;
	int32_t i;
	uint32_t ui;
	int64_t l;
	uint64_t ul;
	float f;
	double d;
};


enum tagENumberType {
	eBYTE = 0,
	eUBYTE = 1,
	eSHORT = (1 << 1),
	eUSHORT = (1 << 1) | 1,
	eINT = (1 << 2),
	eUINT = (1 << 2) | 1,
	eLONG = (1 << 3),
	eULONG = (1 << 3) | 1,
	eFLOAT = (1 << 4),
	eDOUBLE = (1 << 5),
};

enum tagEBinaryOperationType {
	eMULTIPLY,
	eADD,
	eSUBTRACT,
	eDIVIDE,
	ePOW,
};

typedef union tagUNumberValue UNumberValue;
typedef enum tagENumberType ENumberType;
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
	UNumberValue val;
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

class ASTNodeFunction : public ASTNode {
public:
	ASTNodeBlock* body;
	std::string return_type;

	ASTNodeFunction(ASTNodeBlock*, std::string);

	virtual ~ASTNodeFunction();
	virtual llvm::Value* gen_code(CodeGenContext*) override;
	virtual ASTNodeFunction* pass_types(CodeGenContext*, ASTType*) override;
};

#endif /* __AST_NODE_H_ */
