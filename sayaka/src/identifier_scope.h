#ifndef __IDENTIFIER_SCOPE_H_
#define __IDENTIFIER_SCOPE_H_

#include <map>
#include <deque>
#include <tuple>

class ASTType;
namespace llvm {
	class Value;
}

class ASTNodeIdentifier;
typedef std::tuple<ASTType*, llvm::Value*> CodeGenVariable;

class IdentifierScope {
public:
	IdentifierScope();

	void put(std::string, CodeGenVariable*);
	CodeGenVariable* get(std::string);
	bool contains(std::string);
	bool in_top(std::string);
	void push();
	void pop();

	~IdentifierScope();
private:
	std::deque<std::map<std::string, CodeGenVariable*>*> stacks;
};

#endif // __IDENTIFIER_SCOPE_H_
