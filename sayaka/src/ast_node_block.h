#ifndef __AST_NODE_BLOCK_H_
#define __AST_NODE_BLOCK_H_

#include "ast_node.h"

class ASTNodeBlock : public ASTNode {
public:
	std::vector<ASTNode*> statements;

	ASTNodeBlock();
	void push(ASTNode*);

	virtual ~ASTNodeBlock();
	virtual llvm::Value* gen_code(CodeGen*) override;
	virtual ASTNodeBlock* pass_types(ASTType*, IdentifierScope) override;
};

#endif /* __AST_NODE_BLOCK_H_ */
