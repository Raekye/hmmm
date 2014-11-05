#ifndef __AST_NODE_ASSIGNMENT_H_
#define __AST_NODE_ASSIGNMENT_H_

#include "ast_node.h"
#include "ast_node_identifier.h"

class ASTNodeAssignment : public ASTNode {
public:
	ASTNodeIdentifier* lhs;
	ASTNode* rhs;

	ASTNodeAssignment(ASTNodeIdentifier*, ASTNode*);

	virtual ~ASTNodeAssignment();
	virtual llvm::Value* gen_code(CodeGen*) override;
	virtual ASTNodeAssignment* pass_types(ASTType*, IdentifierScope) override;
};

#endif /* __AST_NODE_ASSIGNMENT_H_ */
