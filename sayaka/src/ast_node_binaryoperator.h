#ifndef __AST_NODE_BINARYOPERATOR_H_
#define __AST_NODE_BINARYOPERATOR_H_

#include "ast_node.h"

class ASTNodeBinaryOperator : public ASTNode {
public:
	EBinaryOperationType op;
	ASTNode* lhs;
	ASTNode* rhs;

	ASTNodeBinaryOperator(EBinaryOperationType, ASTNode*, ASTNode*);

	virtual llvm::Value* gen_code(CodeGen*) override;
	virtual ~ASTNodeBinaryOperator();
};

#endif /* __AST_NODE_BINARYOPERATOR_H_ */
