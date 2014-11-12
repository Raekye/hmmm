#include "ast_node.h"

#include <iostream>

ASTNodeBlock::ASTNodeBlock() {
	return;
}

ASTNodeBlock::~ASTNodeBlock() {
	for (std::vector<ASTNode*>::iterator it = this->statements.begin(); it != this->statements.end(); it++) {
		delete *it;
	}
}

void ASTNodeBlock::push(ASTNode* node) {
	this->statements.push_back(node);
}

ASTNodeBlock* ASTNodeBlock::pass_types(CodeGenContext* code_gen_context, ASTType* type) {
	for (std::vector<ASTNode*>::iterator it = this->statements.begin(); it != this->statements.end(); it++) {
		(*it) = (*it)->pass_types(code_gen_context, type);
	}
	this->type = this->statements.back()->type;
	return this;
}

llvm::Value* ASTNodeBlock::gen_code(CodeGenContext* code_gen_context) {
	std::cout << "Generating block" << std::endl;
	llvm::Value* last = NULL;
	for (std::vector<ASTNode*>::iterator it = this->statements.begin(); it != this->statements.end(); it++) {
		last = (*it)->gen_code(code_gen_context);
	}
	return last;
}
