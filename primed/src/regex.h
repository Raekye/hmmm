#ifndef PRIMED_REGEX_H_INCLUDED
#define PRIMED_REGEX_H_INCLUDED

#include <string>
#include <vector>
#include <stack>
#include <tuple>

class RegexAST;
class IRegexASTVisitor;

class RegexParser {
private:
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

	std::vector<RegexAST*>* parse(std::string);
};

class RegexAST {
public:
	virtual ~RegexAST();
	virtual void accept(IRegexASTVisitor*) = 0;
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
	virtual void visit(RegexASTLiteral*) = 0;
	virtual void visit(RegexASTOr*) = 0;
	virtual void visit(RegexASTMultiplication*) = 0;
	virtual void visit(RegexASTRange*) = 0;
};

#endif /* PRIMED_REGEX_H_INCLUDED */
