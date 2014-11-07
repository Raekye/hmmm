#include "ast_type.h"
#include <algorithm>

ASTType::ASTType(std::string name, llvm::Type* llvm_type, bool primitive) {
	this->name = name;
	this->primitive = primitive;
	this->llvm_type = llvm_type;
}

bool ASTType::is_primitive() {
	return this->primitive;
}

bool ASTType::is_signed() {
	if (!this->is_primitive()) {
		throw std::logic_error("Member function is_signed called on non primitive type");
	}
	return this->name[0] != 'U';
}

bool ASTType::is_integral() {
	if (!this->is_primitive()) {
		throw std::logic_error("Member function is_integral called on non primitive type");
	}
	return !this->is_floating();
}

bool ASTType::is_floating() {
	if (!this->is_primitive()) {
		throw std::logic_error("Member function is_floating called on non primitive type");
	}
	return this->name == "Double" || this->name == "Float";
}
