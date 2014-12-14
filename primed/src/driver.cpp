#include <iostream>
#include "regex.h"

class A : public IRegexASTVisitor {
public:
	int32_t indents = 0;
	void f() {
		for (int32_t i = 0; i < indents; i++) {
			std::cout << "\t";
		}
	}
	virtual void visit(RegexASTOr* x) override {
		f();
		std::cout << "or" << std::endl;
		indents++;
		x->left->accept(this);
		f();
		std::cout << "---" << std::endl;
		x->right->accept(this);
		indents--;
		f();
		std::cout << "endor" << std::endl;
	}
	virtual void visit(RegexASTLiteral* x) override {
		f();
		std::cout << "literal: " << (char) x->ch << std::endl;
	}
	virtual void visit(RegexASTMultiplication* x) override {
		f();
		std::cout << "multiply " << x->min << " to " << x->max << ":" << std::endl;
		indents++;
		x->node->accept(this);
		indents--;
		f();
		std::cout << "endmultiply" << std::endl;
	}
	virtual void visit(RegexASTRange* x) override {
		f();
		std::cout << "range: " << (char) x->lower << " to " << (char) x->upper << std::endl;
	}
};

int main() {
	std::cout << "hmmm" << std::endl;
	RegexParser r;
	std::vector<RegexAST*>* node = r.parse("[a-z]*abc+12(4){16,32}");
	if (node != NULL) {
		for (int32_t i = 0; i < node->size(); i++) {
			A a;
			node->operator[](i)->accept(&a);
			delete node->operator[](i);
		}
		delete node;
	} else {
		std::cout << "node was null" << std::endl;
	}
	return 0;
}
