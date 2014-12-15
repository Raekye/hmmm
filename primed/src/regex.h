#ifndef PRIMED_REGEX_H_INCLUDED
#define PRIMED_REGEX_H_INCLUDED

#include <string>
#include <vector>
#include <stack>
#include <tuple>
#include <iostream>

class RegexAST;
class RegexASTChain;
class IRegexASTVisitor;

class RegexParser {
private:
	RegexASTChain* parse_chain();
	RegexAST* parse_toplevel();
	RegexAST* parse_toplevel_nonrecursive();
	RegexAST* parse_parenthesis();
	RegexAST* parse_literal();
	RegexAST* parse_or();
	RegexAST* parse_multiplication();
	RegexAST* parse_group();
	std::tuple<int32_t, int32_t>* parse_multiplication_range();
	int32_t parse_number();
	RegexAST* parse_group_element();
	int32_t* parse_group_literal();
	RegexAST* parse_group_range();

	int32_t buffer_pos();
	void buffer_advance(int32_t);
	int32_t buffer_char(int32_t = 0);
	void buffer_push(int32_t);
	int32_t buffer_pop(int32_t);

	static bool is_special_char(int32_t ch);

public:
	std::string buffer;
	std::stack<int32_t> pos;

	RegexParser();

	RegexASTChain* parse(std::string);
};

class RegexAST {
public:
	virtual ~RegexAST();
	virtual void accept(IRegexASTVisitor*) = 0;
};

class RegexASTChain : public RegexAST {
public:
	std::vector<RegexAST*>* sequence;

	RegexASTChain(std::vector<RegexAST*>*);
	virtual ~RegexASTChain();
	virtual void accept(IRegexASTVisitor*) override;
};

class RegexASTLiteral : public RegexAST {
public:
	int32_t ch;

	RegexASTLiteral(int32_t);
	virtual ~RegexASTLiteral();
	virtual void accept(IRegexASTVisitor*) override;
};

class RegexASTOr : public RegexAST {
public:
	RegexAST* left;
	RegexAST* right;

	RegexASTOr(RegexAST*, RegexAST*);
	virtual ~RegexASTOr();
	virtual void accept(IRegexASTVisitor*) override;
};

class RegexASTMultiplication : public RegexAST {
public:
	int32_t min;
	int32_t max;
	RegexAST* node;

	RegexASTMultiplication(RegexAST*, int32_t, int32_t);
	virtual ~RegexASTMultiplication();
	virtual void accept(IRegexASTVisitor*) override;

	bool is_infinite();
};

class RegexASTRange : public RegexAST {
public:
	int32_t lower;
	int32_t upper;

	RegexASTRange(int32_t, int32_t);
	virtual ~RegexASTRange();
	virtual void accept(IRegexASTVisitor*) override;
};

class IRegexASTVisitor {
public:
	virtual void visit(RegexASTChain*) = 0;
	virtual void visit(RegexASTLiteral*) = 0;
	virtual void visit(RegexASTOr*) = 0;
	virtual void visit(RegexASTMultiplication*) = 0;
	virtual void visit(RegexASTRange*) = 0;
};

class RegexASTPrinter : public IRegexASTVisitor {
public:
	int32_t indents = 0;
	void f() {
		for (int32_t i = 0; i < indents; i++) {
			std::cout << "\t";
		}
	}
	virtual void visit(RegexASTChain* x) override {
		for (int32_t i = 0; i < x->sequence->size(); i++) {
			x->sequence->operator[](i)->accept(this);
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

#endif /* PRIMED_REGEX_H_INCLUDED */
