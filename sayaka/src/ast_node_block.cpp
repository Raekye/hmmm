#include "ast_node_block.h"

#include <iostream>

ASTNodeBlock::ASTNodeBlock() {
	return;
}

ASTNodeBlock::~ASTNodeBlock() {
	for (std::vector<ASTNode*>::iterator it = this->statements.begin(); it != this->statements.end(); it++) {
		delete *it;
	}
}

ASTNodeBlock* ASTNodeBlock::pass_types(ASTType* type, IdentifierScope scope) {
	scope.push();
	for (std::vector<ASTNode*>::iterator it = this->statements.begin(); it != this->statements.end(); it++) {
		(*it) = (*it)->pass_types(type, scope);
	}
	scope.pop();
	return this;
}

void ASTNodeBlock::push(ASTNode* node) {
	this->statements.push_back(node);
}

llvm::Value* ASTNodeBlock::gen_code(CodeGen* code_gen) {
	std::cout << "Generating block..." << std::endl;
	llvm::Value* last = NULL;
	for (std::vector<ASTNode*>::iterator it = this->statements.begin(); it != this->statements.end(); it++) {
		last = (*it)->gen_code(code_gen);
		this->type = (*it)->type;
	}
	return last;
}
