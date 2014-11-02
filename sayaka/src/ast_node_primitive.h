#ifndef __AST_NODE_PRIMITIVE_H_
#define __AST_NODE_PRIMITIVE_H_

#include "ast_node.h"

class ASTNodePrimitive : public ASTNode {
public:
	UNumberValue val;
	std::string str;

	ASTNodePrimitive(std::string);

	virtual llvm::Value* gen_code(CodeGen*) override;
	virtual ~ASTNodePrimitive();
};

#endif /* __AST_NODE_PRIMITIVE_H_ */
