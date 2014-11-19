#include "ast_node.h"

ASTNode::~ASTNode() {
	return;
}

void ASTNode::accept(IASTNodeVisitor* visitor) {
	visitor->visit(this);
}

void IASTNodeVisitor::visit(ASTNode* node) {
	if (ASTNodeIdentifier* x = dynamic_cast<ASTNodeIdentifier*>(node)) {
		this->visit(x);
	}
	if (ASTNodePrimitive* x = dynamic_cast<ASTNodePrimitive*>(node)) {
		this->visit(x);
	}
	if (ASTNodeDeclaration* x = dynamic_cast<ASTNodeDeclaration*>(node)) {
		this->visit(x);
	}
	if (ASTNodeBlock* x = dynamic_cast<ASTNodeBlock*>(node)) {
		this->visit(x);
	}
	if (ASTNodeCast* x = dynamic_cast<ASTNodeCast*>(node)) {
		this->visit(x);
	}
	if (ASTNodeAssignment* x = dynamic_cast<ASTNodeAssignment*>(node)) {
		this->visit(x);
	}
	if (ASTNodeBinaryOperator* x = dynamic_cast<ASTNodeBinaryOperator*>(node)) {
		this->visit(x);
	}
	if (ASTNodeFunctionPrototype* x = dynamic_cast<ASTNodeFunctionPrototype*>(node)) {
		this->visit(x);
	}
	if (ASTNodeFunction* x = dynamic_cast<ASTNodeFunction*>(node)) {
		this->visit(x);
	}
	if (ASTNodeFunctionCall* x = dynamic_cast<ASTNodeFunctionCall*>(node)) {
		this->visit(x);
	}
	if (ASTNodeIfElse* x = dynamic_cast<ASTNodeIfElse*>(node)) {
		this->visit(x);
	}
	throw std::runtime_error("Unknown node type!");
}
