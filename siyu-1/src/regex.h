#ifndef SIYU_REGEX_H_INCLUDED
#define SIYU_REGEX_H_INCLUDED

#include <string>
#include <vector>
#include <set>
#include <stack>
#include <tuple>
#include <iostream>
#include "global.h"
#include "finite_automata.h"

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

	static RegexAST* make_regex_ast_literal(UInt);
	static RegexAST* make_regex_ast_add(std::vector<RegexAST*>*);
	static RegexAST* make_regex_ast_or(RegexAST*, RegexAST*);
	static RegexAST* make_regex_ast_mul(RegexAST*, UInt, UInt);
	static RegexAST* make_regex_ast_range(UInt, UInt);

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
	virtual ~RegexAST();
	virtual void accept(IRegexASTVisitor*) = 0;
	virtual bool nullable() = 0;
	virtual std::vector<RegexAST*> first_pos() = 0;
	virtual std::vector<RegexAST*> last_pos() = 0;
	virtual std::vector<RegexAST*> follow_pos() = 0;
};

class RegexASTEpsilon : public RegexAST {
public:
	void accept(IRegexASTVisitor*) override;
	bool nullable() override;
	std::vector<RegexAST*> first_pos() override;
	std::vector<RegexAST*> last_pos() override;
	std::vector<RegexAST*> follow_pos() override;
};

class RegexASTLiteral : public RegexAST {
public:
	UInt ch;

	RegexASTLiteral(UInt);
	virtual ~RegexASTLiteral();
	void accept(IRegexASTVisitor*) override;
	bool nullable() override;
	std::vector<RegexAST*> first_pos() override;
	std::vector<RegexAST*> last_pos() override;
	std::vector<RegexAST*> follow_pos() override;
};

class RegexASTConcat : public RegexAST {
public:
	RegexAST* curr;
	RegexAST* next;

	RegexASTConcat(RegexAST*, RegexAST*);
	virtual ~RegexASTConcat();
	void accept(IRegexASTVisitor*) override;
	bool nullable() override;
	std::vector<RegexAST*> first_pos() override;
	std::vector<RegexAST*> last_pos() override;
	std::vector<RegexAST*> follow_pos() override;
};

class RegexASTOr : public RegexAST {
public:
	RegexAST* left;
	RegexAST* right;

	RegexASTOr(RegexAST*, RegexAST*);
	virtual ~RegexASTOr();
	void accept(IRegexASTVisitor*) override;
	bool nullable() override;
	std::vector<RegexAST*> first_pos() override;
	std::vector<RegexAST*> last_pos() override;
	std::vector<RegexAST*> follow_pos() override;
};

class RegexASTStar : public RegexAST {
public:
	RegexAST* node;

	RegexASTStar(RegexAST*);
	virtual ~RegexASTStar();
	void accept(IRegexASTVisitor*) override;
	bool nullable() override;
	std::vector<RegexAST*> first_pos() override;
	std::vector<RegexAST*> last_pos() override;
	std::vector<RegexAST*> follow_pos() override;
};

class RegexASTPhony : public RegexAST {
public:
	RegexAST* node;
	RegexASTPhony(RegexAST*);
	void accept(IRegexASTVisitor*) override;
	bool nullable() override;
	std::vector<RegexAST*> first_pos() override;
	std::vector<RegexAST*> last_pos() override;
	std::vector<RegexAST*> follow_pos() override;
};

class IRegexASTVisitor {
public:
	virtual void visit(RegexASTEpsilon*) = 0;
	virtual void visit(RegexASTLiteral*) = 0;
	virtual void visit(RegexASTConcat*) = 0;
	virtual void visit(RegexASTOr*) = 0;
	virtual void visit(RegexASTStar*) = 0;
	virtual void visit(RegexASTPhony*) = 0;
};

class RegexASTPrinter : public IRegexASTVisitor {
public:
	Int indents = 0;
	void f() {
		for (Int i = 0; i < indents; i++) {
			std::cout << "  ";
		}
	}
	void visit(RegexASTEpsilon* x) override {
		f();
		std::cout << "epsilon" << std::endl;
	}
	void visit(RegexASTLiteral* x) override {
		f();
		std::cout << "literal: " << (char) x->ch << std::endl;
	}
	void visit(RegexASTConcat* x) override {
		f();
		std::cout << "concat" << std::endl;
		indents++;
		x->curr->accept(this);
		f();
		std::cout << "---" << std::endl;
		x->next->accept(this);
		indents--;
		f();
		std::cout << "endconcat" << std::endl;
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
	void visit(RegexASTStar* x) override {
		f();
		std::cout << "star" << std::endl;
		indents++;
		x->node->accept(this);
		indents--;
		f();
		std::cout << "endstar" << std::endl;
	}
	void visit(RegexASTPhony* x) override {
		x->node->accept(this);
	}
};

#endif // SIYU_REGEX_H_INCLUDED
