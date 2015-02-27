#ifndef PRIMED_REGEX_H_INCLUDED
#define PRIMED_REGEX_H_INCLUDED

#include <string>
#include <vector>
#include <set>
#include <stack>
#include <tuple>
#include <iostream>
#include "types.h"
#include "finite_automata.h"

typedef NFAState<UInt> RegexNFAState;
typedef NFA<UInt> RegexNFA;

class RegexAST;
class IRegexASTVisitor;

class RegexParser {
private:
	std::string buffer;
	std::stack<Int> pos;

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
	UInt* parse_absolute_literal();

	std::tuple<UInt, UInt>* parse_mul_range();

	RegexAST* parse_group_contents();
	RegexAST* parse_group_element();
	RegexAST* parse_group_range();
	UInt* parse_group_literal();

	UInt* parse_hex_byte();
	UInt* parse_hex_int();
	UInt* parse_dec_int();

	Int buffer_pos();
	void buffer_advance(Int);
	UInt buffer_char(Int = 0);
	void buffer_push(Int);
	Int buffer_pop(Int);

	static bool is_special_char(UInt);
	static bool is_group_special_char(UInt);
	static bool is_hex_digit(UInt);
	static bool is_dec_digit(UInt);

	static const UInt TOKEN_STAR = '*';
	static const UInt TOKEN_PLUS = '+';
	static const UInt TOKEN_QUESTION_MARK = '?';
	static const UInt TOKEN_OR = '|';
	static const UInt TOKEN_DASH = '-';
	static const UInt TOKEN_ESCAPE = '\\';
	static const UInt TOKEN_LPAREN = '(';
	static const UInt TOKEN_RPAREN = ')';
	static const UInt TOKEN_LBRACE = '{';
	static const UInt TOKEN_RBRACE = '}';
	static const UInt TOKEN_LBRACKET = '[';
	static const UInt TOKEN_RBRACKET = ']';
	static const UInt TOKEN_X = 'x';
	static const UInt TOKEN_U = 'u';
	static const UInt TOKEN_T = 't';
	static const UInt TOKEN_N = 'n';
	static const UInt TOKEN_R = 'r';

public:
	RegexAST* parse(std::string);
};

class RegexAST {
public:
	bool terminal;

	RegexAST();
	virtual ~RegexAST();
	virtual void mark_terminal() = 0;
	virtual void accept(IRegexASTVisitor*) = 0;
};

class RegexASTChain : public RegexAST {
public:
	std::vector<RegexAST*> sequence;

	RegexASTChain(std::vector<RegexAST*>);
	virtual ~RegexASTChain();
	virtual void mark_terminal() override;
	virtual void accept(IRegexASTVisitor*) override;
};

class RegexASTLiteral : public RegexAST {
public:
	UInt ch;

	RegexASTLiteral(Int);
	virtual ~RegexASTLiteral();
	virtual void mark_terminal() override;
	virtual void accept(IRegexASTVisitor*) override;
};

class RegexASTOr : public RegexAST {
public:
	RegexAST* left;
	RegexAST* right;

	RegexASTOr(RegexAST*, RegexAST*);
	virtual ~RegexASTOr();
	virtual void mark_terminal() override;
	virtual void accept(IRegexASTVisitor*) override;
};

class RegexASTMultiplication : public RegexAST {
public:
	UInt min;
	UInt max;
	RegexAST* node;

	RegexASTMultiplication(RegexAST*, Int, Int);
	virtual ~RegexASTMultiplication();
	virtual void mark_terminal() override;
	virtual void accept(IRegexASTVisitor*) override;

	bool is_infinite();
};

class RegexASTRange : public RegexAST {
public:
	UInt lower;
	UInt upper;
	std::vector<RegexAST*> nodes;

	RegexASTRange(UInt, UInt);
	virtual ~RegexASTRange();
	virtual void mark_terminal() override;
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

class RegexNFAGenerator : public IRegexASTVisitor {
	RegexNFAState* root;
	RegexNFAState* target_state;
	RegexNFAState* ret;

	RegexNFAState* next_state();
public:
	RegexNFA nfa;

	RegexNFAGenerator();

	void visit(RegexASTChain*) override;
	void visit(RegexASTLiteral*) override;
	void visit(RegexASTOr*) override;
	void visit(RegexASTMultiplication*) override;
	void visit(RegexASTRange*) override;
};

class RegexASTPrinter : public IRegexASTVisitor {
public:
	Int indents = 0;
	void f() {
		for (Int i = 0; i < indents; i++) {
			std::cout << "\t";
		}
	}
	void visit(RegexASTChain* x) override {
		for (Int i = 0; i < x->sequence.size(); i++) {
			x->sequence[i]->accept(this);
		}
	}
	void visit(RegexASTOr* x) override {
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
	void visit(RegexASTLiteral* x) override {
		f();
		std::cout << "literal: " << (char) x->ch << std::endl;
	}
	void visit(RegexASTMultiplication* x) override {
		f();
		std::cout << "multiply " << x->min << " to " << x->max << ":" << std::endl;
		indents++;
		x->node->accept(this);
		indents--;
		f();
		std::cout << "endmultiply" << std::endl;
	}
	void visit(RegexASTRange* x) override {
		f();
		std::cout << "range: " << (char) x->lower << " to " << (char) x->upper << std::endl;
	}
};

#endif /* PRIMED_REGEX_H_INCLUDED */
