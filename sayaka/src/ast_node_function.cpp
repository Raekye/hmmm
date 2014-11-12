#include "ast_node.h"

#include <iostream>

ASTNodeFunction::ASTNodeFunction(ASTNodeFunctionPrototype* prototype, ASTNodeBlock* body) {
	this->prototype = prototype;
	this->body = body;
}

ASTNodeFunction::~ASTNodeFunction() {
	delete this->prototype;
	delete this->body;
}

ASTNodeFunction* ASTNodeFunction::pass_types(CodeGenContext* code_gen_context, ASTType* ignore) {
	this->type = NULL; // TODO;
	code_gen_context->push_scope();
	this->prototype = this->prototype->pass_types(code_gen_context, NULL);
	if (this->body != NULL) {
		this->body = this->body->pass_types(code_gen_context, code_gen_context->ast_types_resolver.get(this->prototype->return_type));
	}
	code_gen_context->pop_scope();
	return this;
}

llvm::Value* ASTNodeFunction::gen_code(CodeGenContext* code_gen_context) {
	std::cout << "Generating function " << this->prototype->function_name << std::endl;
	std::vector<llvm::Type*> arg_types;
	ASTType* type = code_gen_context->ast_types_resolver.get(this->prototype->return_type);
	if (type == NULL) {
		throw std::runtime_error("Unknown type");
	}
	code_gen_context->push_scope();
	llvm::Function* fn = (llvm::Function*) this->prototype->gen_code(code_gen_context);

	if (this->body != NULL) {
		llvm::BasicBlock* basic_block = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", fn);
		code_gen_context->push_block(basic_block);

		llvm::Value* ret_val = this->body->gen_code(code_gen_context);
		code_gen_context->builder.CreateRet(ret_val);
		llvm::verifyFunction(*fn);
		code_gen_context->pop_block();
		fn->dump();
	}
	code_gen_context->pop_scope();
	return fn;
}
