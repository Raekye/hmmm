#ifndef __AST_NODE_CAST_H_
#define __AST_NODE_CAST_H_

#include "ast_node.h"

class ASTNodeCast : public ASTNode {
public:
	ASTNode* val;

	ASTNodeCast(ASTNode*, ASTType*);

	virtual ~ASTNodeCast();
	virtual llvm::Value* gen_code(CodeGen*) override;
	virtual ASTNodeCast* pass_types(ASTType*, IdentifierScope) override;
};

#endif /* __AST_NODE_CAST_H_ */
