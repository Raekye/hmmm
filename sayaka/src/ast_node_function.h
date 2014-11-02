#ifndef __AST_NODE_FUNCTION_H_
#define __AST_NODE_FUNCTION_H_

#include "ast_node.h"

class ASTNodeFunction : public ASTNode {
public:
	ASTNode* body;
	ASTType* return_type;

	ASTNodeFunction(ASTNode*, ASTType*);

	virtual llvm::Value* gen_code(CodeGen*) override;
	virtual ~ASTNodeFunction();
};

#endif /* __AST_NODE_FUNCTION_H_ */
