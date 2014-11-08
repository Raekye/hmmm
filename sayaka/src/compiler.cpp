#include "compiler.h"

#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/ManagedStatic.h>
#include "parser.h"
#include "lexer.h"
#include "ast_node.h"

Compiler::Compiler() {
	return;
}

Compiler::~Compiler() {
	return;
}

ASTNode* Compiler::parse(std::string code) {
	ASTNode* root;
	yyscan_t scanner;
	YY_BUFFER_STATE buffer;
	// (http://flex.sourceforge.net/manual/Reentrant-Overview.html)
	if (yylex_init(&scanner)) {
		return NULL;
	}
	buffer = yy_scan_string(code.c_str(), scanner);
	yyparse(&root, scanner);
	yy_delete_buffer(buffer, scanner);
	yylex_destroy(scanner);
	return root;
}

void Compiler::run_code(ASTNode* root) {
	ASTNodeFunction main_fn((ASTNodeBlock*) root, "Int");
	main_fn.pass_types(&this->code_gen_context, NULL);
	llvm::Function* main_fn_val = (llvm::Function*) main_fn.gen_code(&this->code_gen_context);
	std::cout << "Main fn code:" << std::endl;
	main_fn_val->dump();
	void* fn_ptr = this->execution_engine->getPointerToFunction(main_fn_val);
	int32_t ret = ((int32_t (*)()) fn_ptr)();
	std::cout << "Main fn at " << fn_ptr << "; executed: " << ret << std::endl;
}

void Compiler::initialize() {
	this->execution_engine = llvm::EngineBuilder(this->code_gen_context.module).setErrorStr(&this->execution_engine_error_str).setEngineKind(llvm::EngineKind::JIT).create();
	if (this->execution_engine == NULL) {
		throw std::runtime_error(this->execution_engine_error_str);
	}
}

void Compiler::shutdown() {
	delete this->execution_engine;
}

void Compiler::llvm_initialize() {
	llvm::InitializeNativeTarget();
}

void Compiler::llvm_shutdown() {
	llvm::llvm_shutdown();
}
