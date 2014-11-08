#ifndef __COMPILER_H_
#define __COMPILER_H_

#include <string>
#include "ast_node.h"
#include "code_gen_context.h"

class Compiler {
public:
	llvm::ExecutionEngine* execution_engine;
	CodeGenContext code_gen_context;
	std::string execution_engine_error_str;

	Compiler();
	void initialize();
	void shutdown();
	void llvm_initialize();
	void llvm_shutdown();

	ASTNode* parse(std::string);
	void run_code(ASTNode*);

	virtual ~Compiler();
};

#endif /* __COMPILER_H_ */
