#include "ast_node.h"

ASTNodeFunctionPrototype::ASTNodeFunctionPrototype(std::string return_type, std::string function_name, std::vector<ASTNodeDeclaration*>* args) {
	this->return_type = return_type;
	this->function_name = function_name;
	this->args = args;
}

ASTNodeFunctionPrototype::~ASTNodeFunctionPrototype() {
	for (std::vector<ASTNodeDeclaration*>::iterator it = this->args->begin(); it != this->args->end(); it++) {
		delete *it;
	}
	delete this->args;
}

ASTNodeFunctionPrototype* ASTNodeFunctionPrototype::pass_types(CodeGenContext* code_gen_context, ASTType* type) {
	this->type = code_gen_context->ast_types_resolver.int_ty(); // TODO
	for (std::vector<ASTNodeDeclaration*>::iterator it = this->args->begin(); it != this->args->end(); it++) {
		*it = (*it)->pass_types(code_gen_context, NULL);
	}
	return this;
}

llvm::Value* ASTNodeFunctionPrototype::gen_code(CodeGenContext* code_gen_context) {
	std::cout << "Generating prototype" << std::endl;
	std::vector<llvm::Type*> args_types_list;
	for (std::vector<ASTNodeDeclaration*>::iterator it = this->args->begin(); it != this->args->end(); it++) {
		args_types_list.push_back(code_gen_context->ast_types_resolver.get((*it)->type_name)->llvm_type);
	}
	llvm::FunctionType* ft = llvm::FunctionType::get(code_gen_context->ast_types_resolver.get(this->return_type)->llvm_type, args_types_list, false);
	llvm::Function* fn = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, this->function_name, code_gen_context->module);
	int i = 0;
	for (llvm::Function::arg_iterator it = fn->arg_begin(); it != fn->arg_end(); it++) {
		ASTNodeDeclaration* var = this->args->operator[](i);
		it->setName(var->var_name);
		code_gen_context->scope.put(var->var_name, new CodeGenVariable(code_gen_context->ast_types_resolver.get(var->type_name), it));
		i++;
	}
	return fn;
	//return llvm::ConstantInt::get(code_gen_context->ast_types_resolver.int_ty()->llvm_type, 0, true);
}

