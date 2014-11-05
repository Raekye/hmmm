#ifndef __AST_NODE_PRIMITIVE_H_
#define __AST_NODE_PRIMITIVE_H_

#include "ast_node.h"

class ASTNodePrimitive : public ASTNode {
public:
	UNumberValue val;
	std::string str;

	ASTNodePrimitive(std::string);

	virtual ~ASTNodePrimitive();
	virtual llvm::Value* gen_code(CodeGen*) override;
	virtual ASTNodePrimitive* pass_types(ASTType*, IdentifierScope) override;
};

#endif /* __AST_NODE_PRIMITIVE_H_ */
