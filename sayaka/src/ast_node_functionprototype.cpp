#include "ast_node.h"

ASTNodeFunctionPrototype::ASTNodeFunctionPrototype(std::string return_type, std::string function_name, std::vector<std::tuple<std::string, std::string>>* args) {
	this->return_type = return_type;
	this->function_name = function_name;
	this->args = args;
}

ASTNodeFunctionPrototype::~ASTNodeFunctionPrototype() {
	delete this->args;
}

ASTNodeFunctionPrototype* ASTNodeFunctionPrototype::pass_types(CodeGenContext* code_gen_context, ASTType* type) {
	this->type = code_gen_context->ast_types_resolver.int_ty();
	return this;
}

llvm::Value* ASTNodeFunctionPrototype::gen_code(CodeGenContext* code_gen_context) {
	std::vector<llvm::Type*> args_types_list;
	for (std::vector<std::tuple<std::string, std::string>>::iterator it = this->args->begin(); it != this->args->end(); it++) {
		args_types_list.push_back(code_gen_context->ast_types_resolver.get(std::get<0>(*it))->llvm_type);
	}
	llvm::FunctionType* ft = llvm::FunctionType::get(code_gen_context->ast_types_resolver.get(this->return_type)->llvm_type, args_types_list, false);
	llvm::Function* fn = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, this->function_name, code_gen_context->module);
	return llvm::ConstantInt::get(code_gen_context->ast_types_resolver.int_ty()->llvm_type, 0, true);
}

