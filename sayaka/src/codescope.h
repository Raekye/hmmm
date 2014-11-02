#ifndef __CODESCOPE_H_
#define __CODESCOPE_H_

#include <map>
#include <deque>
#include <llvm/IR/Value.h>

class ASTNode;

class CodeScope {
public:
	CodeScope();
	~CodeScope();
	void put(std::string, ASTNode*);
	ASTNode* get(std::string);
	bool contains(std::string);
	void push();
	void pop();
private:
	std::deque<std::map<std::string, ASTNode*>*> stacks;
};

#endif // __CODESCOPE_H_
