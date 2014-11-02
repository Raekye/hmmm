#ifndef __AST_NODE_CAST_H_
#define __AST_NODE_CAST_H_

#include "ast_node.h"

class ASTNodeCast : public ASTNode {
public:
	ASTType* target_type;
	ASTNode* val;

	ASTNodeCast(ASTNode*, ASTType*);

	virtual llvm::Value* gen_code(CodeGen*) override;
	virtual ~ASTNodeCast();
};

#endif /* __AST_NODE_CAST_H_ */
