#include "ast_node_identifier.h"

#include <iostream>
#include <sstream>

ASTNodeIdentifier::ASTNodeIdentifier(std::string name, ASTType* type) {
	this->name = name;
	this->type = type;
}

ASTNodeIdentifier::~ASTNodeIdentifier() {
	return;
}

ASTNodeIdentifier* ASTNodeIdentifier::pass_types(ASTType* type, IdentifierScope scope) {
	return this;
}

llvm::Value* ASTNodeIdentifier::gen_code(CodeGen* code_gen) {
	std::cout << "Generating identifier " << this->name << std::endl;
	ASTNode* val = code_gen->scope.get(this->name);
	if (val == NULL) {
		std::stringstream ss;
		ss << "Undeclared variable " << this->name;
		throw std::runtime_error(ss.str());
	}
	//return new llvm::LoadInst(val->value, "", false, code_gen->current_block());
	return NULL;
}
