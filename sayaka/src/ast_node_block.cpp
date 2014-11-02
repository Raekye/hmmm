#include "ast_node_block.h"

#include <iostream>

ASTNodeBlock::ASTNodeBlock() {
	return;
}

void ASTNodeBlock::push(ASTNode* node) {
	this->statements.push_back(node);
}

ASTNodeBlock::~ASTNodeBlock() {
	return;
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
