#ifndef __AST_NODE_IDENTIFIER_H_
#define __AST_NODE_IDENTIFIER_H_

#include "ast_node.h"

class ASTNodeIdentifier : public ASTNode {
public:
	std::string name;

	ASTNodeIdentifier(std::string, ASTType*);

	virtual ~ASTNodeIdentifier();
	virtual llvm::Value* gen_code(CodeGen*) override;
	virtual ASTNodeIdentifier* pass_types(ASTType*, IdentifierScope) override;
};

#endif /* __AST_NODE_IDENTIFIER_H_ */
