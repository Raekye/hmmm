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

	bool is_infinite() {
		return this->max == 0;
	}
};

class RegexASTGroup : public RegexAST {
public:
	static UInt const UNICODE_MAX;

	typedef std::pair<UInt, UInt> Range;
	class RangeList {
	public:
		Range range;
		std::unique_ptr<RangeList> next;

		void flatten(std::vector<Range>* l) {
			l->push_back(this->range);
			if (this->next != nullptr) {
				this->next->flatten(l);
			}
		}
	};

	bool negate;
	std::unique_ptr<RangeList> span;

	RegexASTGroup(bool, std::unique_ptr<RangeList>);
	void accept(IRegexASTVisitor*) override;
	void add_range(UInt, UInt);
	void flatten(std::vector<Range>*);

	static void merge(std::vector<Range>*, std::vector<Range>*);
	static void complement(std::vector<Range>*, std::vector<Range>*);
};

class IRegexASTVisitor {
public:
	virtual void visit(RegexASTLiteral*) = 0;
	virtual void visit(RegexASTChain*) = 0;
	virtual void visit(RegexASTOr*) = 0;
	virtual void visit(RegexASTMultiplication*) = 0;
	virtual void visit(RegexASTGroup*) = 0;
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
	void visit(RegexASTGroup*) override;
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
	void visit(RegexASTGroup* x) override {
		f();
		std::cout << "range " << x->negate << std::endl;
		indents++;
		for (RegexASTGroup::RangeList* r = x->span.get(); r != nullptr; r = r->next.get()) {
			f();
			std::cout << r->range.first << " to " << r->range.second << std::endl;
		}
		indents--;
		f();
		std::cout << "endrange" << std::endl;
	}
};

#endif /* MIDORI_REGEX_H_INCLUDED */
