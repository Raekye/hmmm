#ifndef __AST_NODE_H_
#define __AST_NODE_H_

#include <iostream>
#include <llvm/IR/Value.h>
#include "ast_type.h"
#include "codegen.h"

class ASTNode {
public:
	ASTType* type;
	llvm::Value* value;

	virtual ~ASTNode() = 0;
	virtual llvm::Value* gen_code(CodeGen*) = 0;
};

#endif /* __AST_NODE_H_ */
