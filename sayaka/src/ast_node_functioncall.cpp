#include "ast_node.h"

ASTNodeFunctionCall::ASTNodeFunctionCall(std::string function_name, std::vector<ASTNode*>* args) {
	this->function_name = function_name;
	this->args = args;
}

ASTNodeFunctionCall::~ASTNodeFunctionCall() {
	for (std::vector<ASTNode*>::iterator it = this->args->begin(); it != this->args->end(); it++) {
		delete *it;
	}
	delete this->args;
}

ASTNodeFunctionCall* ASTNodeFunctionCall::pass_types(CodeGenContext* code_gen_context, ASTType* type) {
	this->type = code_gen_context->ast_types_resolver.double_ty();
	for (std::vector<ASTNode*>::iterator it = this->args->begin(); it != this->args->end(); it++) {
		*it = (*it)->pass_types(code_gen_context, type);
	}
	return this;
}

llvm::Value* ASTNodeFunctionCall::gen_code(CodeGenContext* code_gen_context) {
	llvm::Function* callee = code_gen_context->module->getFunction(this->function_name);
	if (callee == NULL) {
		throw std::runtime_error("Unknown function called");
	}
	std::vector<llvm::Value*> argsv;
	for (std::vector<ASTNode*>::iterator it = this->args->begin(); it != this->args->end(); it++) {
		argsv.push_back((*it)->gen_code(code_gen_context));
	}
	return code_gen_context->builder.CreateCall(callee, argsv, "calltmp");
}
