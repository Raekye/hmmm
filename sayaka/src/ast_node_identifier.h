#ifndef __AST_NODE_IDENTIFIER_H_
#define __AST_NODE_IDENTIFIER_H_

#include "ast_node.h"

class ASTNodeIdentifier : public ASTNode {
public:
	std::string name;

	ASTNodeIdentifier(std::string, ASTType*);

	virtual llvm::Value* gen_code(CodeGen*) override;
	virtual ~ASTNodeIdentifier();
};

#endif /* __AST_NODE_IDENTIFIER_H_ */
