#ifndef SIYU_REGEX_H_INCLUDED
#define SIYU_REGEX_H_INCLUDED

#include <string>
#include <vector>
#include <set>
#include <stack>
#include <tuple>
#include <iostream>
#include <memory>
#include "global.h"
#include "finite_automata.h"

typedef NFAState<UInt, std::string> RegexNFAState;
typedef NFA<UInt, std::string> RegexNFA;
typedef DFAState<UInt, std::string> RegexDFAState;
typedef DFA<UInt, std::string> RegexDFA;

class RegexAST;
class IRegexASTVisitor;

class RegexParser {
private:
	std::string buffer;
	std::stack<Int> pos;

	Int buffer_pos();
	void buffer_advance(Int);
	UInt buffer_char(Int = 0);
	void buffer_push(Int);
	Int buffer_pop(Int);

	static std::unique_ptr<RegexAST> make_regex_ast_literal(UInt);
	static std::unique_ptr<RegexAST> make_regex_ast_chain(std::vector<std::unique_ptr<RegexAST>>*);
	static std::unique_ptr<RegexAST> make_regex_ast_or(std::unique_ptr<RegexAST>*, std::unique_ptr<RegexAST>*);
	static std::unique_ptr<RegexAST> make_regex_ast_multiplication(std::unique_ptr<RegexAST>*, UInt, UInt);
	static std::unique_ptr<RegexAST> make_regex_ast_range(UInt, UInt);

	std::unique_ptr<RegexAST> parse_toplevel();
	std::unique_ptr<RegexAST> parse_lr_or();
	std::unique_ptr<RegexAST> parse_not_lr_or();
	std::unique_ptr<RegexAST> parse_lr_add();
	std::unique_ptr<RegexAST> parse_not_lr_add();
	std::unique_ptr<RegexAST> parse_lr_mul();
	std::unique_ptr<RegexAST> parse_not_lr_mul();
	std::unique_ptr<RegexAST> parse_not_lr();
	std::unique_ptr<RegexAST> parse_parentheses();
	std::unique_ptr<RegexAST> parse_literal();
	std::unique_ptr<RegexAST> parse_group();
	UInt* parse_absolute_literal();

	std::tuple<UInt, UInt>* parse_mul_range();

	std::unique_ptr<RegexAST> parse_group_contents();
	std::unique_ptr<RegexAST> parse_group_element();
	std::unique_ptr<RegexAST> parse_group_range();
	UInt* parse_group_literal();

	UInt* parse_hex_byte();
	UInt* parse_hex_int();
	UInt* parse_dec_int();

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
	std::unique_ptr<RegexAST> parse(std::string);
};

class RegexAST {
public:
	virtual ~RegexAST();
	virtual void accept(IRegexASTVisitor*) = 0;
};

class RegexASTLiteral : public RegexAST {
public:
	UInt ch;

	RegexASTLiteral(UInt);
	void accept(IRegexASTVisitor*) override;
};

class RegexASTChain : public RegexAST {
public:
	std::vector<std::unique_ptr<RegexAST>> sequence;

	RegexASTChain(std::vector<std::unique_ptr<RegexAST>>);
	void accept(IRegexASTVisitor*) override;
};

class RegexASTOr : public RegexAST {
public:
	std::unique_ptr<RegexAST> left;
	std::unique_ptr<RegexAST> right;

	RegexASTOr(std::unique_ptr<RegexAST>, std::unique_ptr<RegexAST>);
	void accept(IRegexASTVisitor*) override;
};

class RegexASTMultiplication : public RegexAST {
public:
	std::unique_ptr<RegexAST> node;
	UInt min;
	UInt max;

	RegexASTMultiplication(std::unique_ptr<RegexAST>, Int, Int);
	void accept(IRegexASTVisitor*) override;

	bool is_infinite();
};

class RegexASTRange : public RegexAST {
public:
	UInt lower;
	UInt upper;

	RegexASTRange(UInt, UInt);
	void accept(IRegexASTVisitor*) override;
};

class IRegexASTVisitor {
public:
	virtual void visit(RegexASTLiteral*) = 0;
	virtual void visit(RegexASTChain*) = 0;
	virtual void visit(RegexASTOr*) = 0;
	virtual void visit(RegexASTMultiplication*) = 0;
	virtual void visit(RegexASTRange*) = 0;
};

class RegexNFAGenerator : public IRegexASTVisitor {
	RegexNFAState* root;
	RegexNFAState* target_state;

	RegexNFAState* next_state();
public:
	RegexNFA nfa;

	RegexNFAGenerator();
	void reset();
	void new_rule(std::string);

	void visit(RegexASTLiteral*) override;
	void visit(RegexASTChain*) override;
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
	void visit(RegexASTLiteral* x) override {
		f();
		std::cout << "literal: " << (char) x->ch << std::endl;
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

#endif /* SIYU_REGEX_H_INCLUDED */
