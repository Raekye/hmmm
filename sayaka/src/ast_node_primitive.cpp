#include "ast_node.h"

#include <iostream>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

ASTNodePrimitive::ASTNodePrimitive(std::string str) {
	this->str = str;
	boost::replace_all(str, "_", "");
	boost::algorithm::to_upper(str);
}

ASTNodePrimitive::~ASTNodePrimitive() {
	return;
}

ASTNodePrimitive* ASTNodePrimitive::pass_types(CodeGenContext* code_gen_context, ASTType* type) {
	if (!type->is_primitive()) {
		throw std::runtime_error("I'm doing the best I can!");
	}
	this->type = type;
	return this;
}

llvm::Value* ASTNodePrimitive::gen_code(CodeGenContext* code_gen_context) {
	std::cout << "Generating primitive" << std::endl;
	if (this->type == code_gen_context->ast_types_resolver.double_ty()) {
		return llvm::ConstantFP::get(this->type->llvm_type, boost::lexical_cast<double>(this->str));
	} else if (this->type == code_gen_context->ast_types_resolver.float_ty()) {
		return llvm::ConstantFP::get(this->type->llvm_type, boost::lexical_cast<float>(this->str));
	}
	if (this->type->is_signed()) {
		return llvm::ConstantInt::get(this->type->llvm_type, boost::lexical_cast<int64_t>(this->str), true);
	} else {
		return llvm::ConstantInt::get(this->type->llvm_type, boost::lexical_cast<uint64_t>(this->str), false);
	}
}
