#include "ast_node_primitive.h"

#include <iostream>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

ASTNodePrimitive::ASTNodePrimitive(std::string str) {
	this->str = str;
	boost::replace_all(str, "_", "");
	boost::algorithm::to_upper(str);
	this->type = ASTType::long_ty();
}

ASTNodePrimitive::~ASTNodePrimitive() {
	return;
}

llvm::Value* ASTNodePrimitive::gen_code(CodeGen* code_gen) {
	std::cout << "Generating primitve number " << this->type->name << "..." << std::endl;
	if (this->type == ASTType::double_ty()) {
		return llvm::ConstantFP::get(this->type->llvm_type, boost::lexical_cast<double>(this->str));
	} else if (this->type == ASTType::float_ty()) {
		return llvm::ConstantFP::get(this->type->llvm_type, boost::lexical_cast<float>(this->str));
	}
	if (this->type->is_signed()) {
		return llvm::ConstantInt::get(this->type->llvm_type, boost::lexical_cast<int64_t>(this->str), true);
	} else {
		return llvm::ConstantInt::get(this->type->llvm_type, boost::lexical_cast<uint64_t>(this->str), false);
	}
}
