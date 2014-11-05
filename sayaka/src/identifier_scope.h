#ifndef __IDENTIFIER_SCOPE_H_
#define __IDENTIFIER_SCOPE_H_

#include <map>
#include <deque>
#include <llvm/IR/Value.h>

class ASTNodeIdentifier;

class IdentifierScope {
public:
	IdentifierScope();

	void put(std::string, ASTNodeIdentifier*);
	ASTNodeIdentifier* get(std::string);
	bool contains(std::string);
	bool in_top(std::string);
	void push();
	void pop();

	~IdentifierScope();
private:
	std::deque<std::map<std::string, ASTNodeIdentifier*>*> stacks;
};

#endif // __IDENTIFIER_SCOPE_H_
