#ifndef __CODESCOPE_H_
#define __CODESCOPE_H_

#include <map>
#include <deque>
#include <llvm/IR/Value.h>
#include "node.h"

class CodeScope {
public:
	CodeScope();
	~CodeScope();
	void put(std::string, NExpression*);
	NExpression* get(std::string);
	bool contains(std::string);
	void push();
	void pop();
private:
	std::deque<std::map<std::string, NExpression*>*> stacks;
};

#endif // __CODESCOPE_H_
