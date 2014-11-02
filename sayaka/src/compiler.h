#ifndef __COMPILER_H_
#define __COMPILER_H_

#include <string>
#include "ast_node.h"

class Compiler {
public:
	Compiler();
	ASTNode* parse(std::string);
	void run_code(ASTNode*);

	virtual ~Compiler();

	static void initialize();
};

#endif /* __COMPILER_H_ */
