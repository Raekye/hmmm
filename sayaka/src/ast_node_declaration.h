#ifndef __AST_NODE_DECLARATION_H_
#define __AST_NODE_DECLARATION_H_

#include "ast_node.h"
#include "ast_node_identifier.h"

class ASTNodeDeclaration : public ASTNode {
public:
	ASTNodeIdentifier* var_name;

	ASTNodeDeclaration(ASTType*, ASTNodeIdentifier*);

	virtual llvm::Value* gen_code(CodeGen*) override;
	virtual ~ASTNodeDeclaration();
};

#endif /* __AST_NODE_DECLARATION_H_ */
