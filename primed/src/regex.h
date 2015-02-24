#ifndef PRIMED_REGEX_H_INCLUDED
#define PRIMED_REGEX_H_INCLUDED

#include <string>
#include <vector>
#include <stack>
#include <tuple>
#include <iostream>
#include "dfa.h"

class RegexAST;
class IRegexASTVisitor;

class RegexParser {
private:
	RegexAST* parse_toplevel();
	RegexAST* parse_lr_or();
	RegexAST* parse_not_lr_or();
	RegexAST* parse_lr_add();
	RegexAST* parse_not_lr_add();
	RegexAST* parse_lr_mul();
	RegexAST* parse_not_lr_mul();
	RegexAST* parse_not_lr();
	RegexAST* parse_parentheses();
	RegexAST* parse_literal();
	RegexAST* parse_group();
	uint32_t* parse_absolute_literal();

	std::tuple<int32_t, int32_t>* parse_mul_range();

	RegexAST* parse_group_contents();
	RegexAST* parse_group_element();
	RegexAST* parse_group_range();
	uint32_t* parse_group_literal();

	uint32_t* parse_hex_byte();
	uint32_t* parse_hex_int();
	uint32_t* parse_dec_int();

	int32_t buffer_pos();
	void buffer_advance(int32_t);
	uint32_t buffer_char(int32_t = 0);
	void buffer_push(int32_t);
	int32_t buffer_pop(int32_t);

	static bool is_special_char(uint32_t);
	static bool is_group_special_char(uint32_t);
	static bool is_hex_digit(uint32_t);
	static bool is_dec_digit(uint32_t);

	static const int32_t TOKEN_STAR = '*';
	static const int32_t TOKEN_PLUS = '+';
	static const int32_t TOKEN_QUESTION_MARK = '?';
	static const int32_t TOKEN_OR = '|';
	static const int32_t TOKEN_DASH = '-';
	static const int32_t TOKEN_ESCAPE = '\\';
	static const int32_t TOKEN_LPAREN = '(';
	static const int32_t TOKEN_RPAREN = ')';
	static const int32_t TOKEN_LBRACE = '{';
	static const int32_t TOKEN_RBRACE = '}';
	static const int32_t TOKEN_LBRACKET = '[';
	static const int32_t TOKEN_RBRACKET = ']';
	static const int32_t TOKEN_X = 'x';
	static const int32_t TOKEN_U = 'u';
	static const int32_t TOKEN_T = 't';
	static const int32_t TOKEN_N = 'n';
	static const int32_t TOKEN_R = 'r';

public:
	std::string buffer;
	std::stack<int32_t> pos;

	RegexParser();

	RegexAST* parse(std::string);
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
	uint32_t ch;

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
	uint32_t lower;
	uint32_t upper;

	RegexASTRange(uint32_t, uint32_t);
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

class RegexDFAGenerator : public IRegexASTVisitor {
	DFAState<uint32_t>* root;
	DFAState<uint32_t>* ret;
	void visit(RegexASTChain*) override;
	void visit(RegexASTLiteral*) override;
	void visit(RegexASTOr*) override;
	void visit(RegexASTMultiplication*) override;
	void visit(RegexASTRange*) override;
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
