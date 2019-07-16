#ifndef MIDORI_REGEX_H_INCLUDED
#define MIDORI_REGEX_H_INCLUDED

#include <string>
#include <vector>
#include <set>
#include <stack>
#include <tuple>
#include <iostream>
#include <memory>
#include "global.h"
#include "finite_automata.h"

typedef NFAState<Long, std::string> RegexNFAState;
typedef NFA<Long, std::string> RegexNFA;
typedef DFAState<Long, std::string> RegexDFAState;
typedef DFA<Long, std::string> RegexDFA;

class RegexAST;
class IRegexASTVisitor;

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

	inline bool is_infinite() {
		return this->max == 0;
	}
};

class RegexASTRange : public RegexAST {
public:
	UInt lower;
	UInt upper;

	RegexASTRange(UInt, UInt);
	void accept(IRegexASTVisitor*) override;
};

class RegexASTNot : public RegexAST {
public:
	std::unique_ptr<RegexAST> node;

	RegexASTNot(std::unique_ptr<RegexAST>);
	void accept(IRegexASTVisitor*) override;
};

class RegexASTWildcard : public RegexAST {
public:
	RegexASTWildcard();
	void accept(IRegexASTVisitor*) override;
};

class IRegexASTVisitor {
public:
	virtual void visit(RegexASTLiteral*) = 0;
	virtual void visit(RegexASTChain*) = 0;
	virtual void visit(RegexASTOr*) = 0;
	virtual void visit(RegexASTMultiplication*) = 0;
	virtual void visit(RegexASTRange*) = 0;
	virtual void visit(RegexASTNot*) = 0;
	virtual void visit(RegexASTWildcard*) = 0;
};

class RegexNFAGenerator : public IRegexASTVisitor {
	RegexNFAState* root;
	RegexNFAState* target_state;
	bool inversion;

	RegexNFAState* next_state();
	inline std::map<Long, std::vector<RegexNFAState*>>* transitions() {
		return this->inversion ? &(this->root->not_states) : &(this->root->next_states);
	}
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
	void visit(RegexASTNot*) override;
	void visit(RegexASTWildcard*) override;
};

// TODO: why no inline compiles?
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
		for (size_t i = 0; i < x->sequence.size(); i++) {
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
	void visit(RegexASTNot* x) override {
		f();
		std::cout << "not:" << std::endl;
		indents++;
		x->node->accept(this);
		indents--;
		f();
		std::cout << "endnot" << std::endl;
	}
	void visit(RegexASTWildcard* x) override {
		(void) x;
		f();
		std::cout << "wildcard" << std::endl;
	}
};

#endif /* MIDORI_REGEX_H_INCLUDED */
