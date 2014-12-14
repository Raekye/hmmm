#include "regex.h"
#include <cctype>
#include <iostream>

RegexASTChain::RegexASTChain(std::vector<RegexAST*>* sequence) {
	this->sequence = sequence;
}

RegexASTLiteral::RegexASTLiteral(int32_t ch) {
	this->ch = ch;
}

RegexASTOr::RegexASTOr(RegexAST* left, RegexAST* right) {
	this->left = left;
	this->right = right;
}

RegexASTMultiplication::RegexASTMultiplication(RegexAST* node, int32_t min, int32_t max) {
	this->node = node;
	this->min = min;
	this->max = max;
}

RegexASTRange::RegexASTRange(int32_t lower, int32_t upper) {
	this->lower = lower;
	this->upper = upper;
}

RegexAST::~RegexAST() {
	return;
}

RegexASTChain::~RegexASTChain() {
	for (int32_t i = 0; i < this->sequence->size(); i++) {
		delete this->sequence->operator[](i);
	}
	delete this->sequence;
}

RegexASTLiteral::~RegexASTLiteral() {
	return;
}

RegexASTOr::~RegexASTOr() {
	delete this->left;
	delete this->right;
}

RegexASTMultiplication::~RegexASTMultiplication() {
	delete this->node;
}

RegexASTRange::~RegexASTRange() {
	return;
}

void RegexASTChain::accept(IRegexASTVisitor* visitor) {
	visitor->visit(this);
}

void RegexASTLiteral::accept(IRegexASTVisitor* visitor) {
	visitor->visit(this);
}

void RegexASTOr::accept(IRegexASTVisitor* visitor) {
	visitor->visit(this);
}

void RegexASTMultiplication::accept(IRegexASTVisitor* visitor) {
	visitor->visit(this);
}

void RegexASTRange::accept(IRegexASTVisitor* visitor) {
	visitor->visit(this);
}

#pragma mark - Regex parser
RegexParser::RegexParser() {
	return;
}

RegexAST* RegexParser::parse(std::string str) {
	this->buffer = str;
	this->pos = std::stack<int32_t>();
	this->pos.push(0);
	RegexAST* regex = this->parse_chain();
	if (this->buffer_pos() != str.length()) {
		delete regex;
		return NULL;
	}
	return regex;
}

RegexAST* RegexParser::parse_chain() {
	std::vector<RegexAST*>* sequence = new std::vector<RegexAST*>();
	RegexAST* first = this->parse_toplevel();
	if (first) {
		sequence->push_back(first);
		while (RegexAST* node = this->parse_toplevel()) {
			sequence->push_back(node);
		}
	} else {
		delete sequence;
		return NULL;
	}
	return new RegexASTChain(sequence);
}

RegexAST* RegexParser::parse_toplevel() {
	RegexAST* node = NULL;
	if ((node = this->parse_multiplication())) {
		return node;
	} else if ((node = this->parse_or())) {
		return node;
	} else if ((node = this->parse_toplevel_nonrecursive())) {
		return node;
	}
	return NULL;
}

RegexAST* RegexParser::parse_toplevel_nonrecursive() {
	RegexAST* node = NULL;
	if ((node = this->parse_parenthesis())) {
		return node;
	} else if ((node = RegexParser::parse_literal())) {
		return node;
	} else if ((node = RegexParser::parse_group())) {
		return node;
	}
	return NULL;
}

RegexAST* RegexParser::parse_parenthesis() {
	if (this->buffer_char() != '(') {
		return NULL;
	}
	this->buffer_advance(1);
	RegexAST* node = this->parse_chain();
	if (!node) {
		this->buffer_pop(1);
		return NULL;
	}
	if (this->buffer_char() != ')') {
		delete node;
		this->buffer_pop(2);
		return NULL;
	}
	this->buffer_push(this->buffer_pop(2) + 1);
	return node;
}

RegexAST* RegexParser::parse_literal() {
	int32_t ch = this->buffer_char();
	if (ch == 0) {
		return NULL;
	}
	if (ch == '\\') {
		ch = this->buffer_char(1);
		if (RegexParser::is_special_char(ch)) {
			this->buffer_advance(2);
			return new RegexASTLiteral(ch);
		}
	}
	if (RegexParser::is_special_char(ch)) {
		return NULL;
	}
	this->buffer_advance(1);
	return new RegexASTLiteral(ch);
}

RegexAST* RegexParser::parse_or() {
	RegexAST* left = this->parse_toplevel_nonrecursive();
	if (!left) {
		return NULL;
	}
	if (this->buffer_char() != '|') {
		delete left;
		this->buffer_pop(1);
		return NULL;
	}
	this->buffer_advance(1);
	RegexAST* right = this->parse_toplevel();
	if (!right) {
		delete left;
		this->buffer_pop(2);
		return NULL;
	}
	this->buffer_push(this->buffer_pop(3));
	return new RegexASTOr(left, right);
}

RegexAST* RegexParser::parse_multiplication() {
	RegexAST* node = this->parse_toplevel_nonrecursive();
	if (!node) {
		return NULL;
	}
	if (this->buffer_char() == '*') {
		this->buffer_push(this->buffer_pop(1) + 1);
		return new RegexASTMultiplication(node, 0, 0);
	} else if (this->buffer_char() == '?') {
		this->buffer_push(this->buffer_pop(1) + 1);
		return new RegexASTMultiplication(node, 0, 1);
	} else if (this->buffer_char() == '+') {
		this->buffer_push(this->buffer_pop(1) + 1);
		return new RegexASTMultiplication(node, 1, 0);
	} else {
		if (std::tuple<int32_t, int32_t>* range = this->parse_multiplication_range()) {
			this->buffer_push(this->buffer_pop(2));
			RegexAST* node_prime = new RegexASTMultiplication(node, std::get<0>(*range), std::get<1>(*range));
			delete range;
			return node_prime;
		}
	}
	delete node;
	this->buffer_pop(1);
	return NULL;
}

RegexAST* RegexParser::parse_group() {
	if (this->buffer_char() != '[') {
		return NULL;
	}
	this->buffer_advance(1);
	RegexAST* first = this->parse_group_element();
	if (!first) {
		this->buffer_pop(1);
		return NULL;
	}
	RegexAST* second = this->parse_group_element();
	if (!second) {
		if (this->buffer_char() != ']') {
			delete first;
			this->buffer_pop(2);
			return NULL;
		}
		this->buffer_push(this->buffer_pop(2) + 1);
		return first;
	}
	int32_t delta = 3;
	RegexASTOr* aggregation = new RegexASTOr(first, second);
	RegexASTOr* tail = aggregation;
	while (RegexAST* node = this->parse_group_element()) {
		RegexASTOr* tail_prime = new RegexASTOr(tail->right, node);
		tail->right = tail_prime;
		tail = tail_prime;
		delta++;
	}
	if (this->buffer_char() != ']') {
		delete aggregation;
		this->buffer_pop(delta);
		return NULL;
	}
	this->buffer_push(this->buffer_pop(delta) + 1);
	return aggregation;
}

std::tuple<int32_t, int32_t>* RegexParser::parse_multiplication_range() {
	if (this->buffer_char() != '{') {
		return NULL;
	}
	this->buffer_advance(1);
	int32_t lower = this->parse_number();
	if (lower < 0) {
		this->buffer_pop(1);
		return NULL;
	}
	if (this->buffer_char() != ',') {
		this->buffer_pop(2);
		return NULL;
	}
	this->buffer_advance(1);
	int32_t upper = this->parse_number();
	if (upper < 0) {
		this->buffer_pop(3);
		return NULL;
	}
	if (this->buffer_char() != '}') {
		this->buffer_pop(4);
		return NULL;
	}
	this->buffer_push(this->buffer_pop(4) + 1);
	return new std::tuple<int32_t, int32_t>(lower, upper);
}

int32_t RegexParser::parse_number() {
	int32_t ch = this->buffer_char();
	if (!std::isdigit(ch)) {
		return -1;
	}
	int32_t delta = 1;
	int32_t x = ch - '0';
	while (true) {
		int32_t ch = this->buffer_char(delta);
		if (!std::isdigit(ch)) {
			break;
		}
		x = x * 10 + (ch - '0');
		delta++;
	}
	this->buffer_advance(delta);
	return x;
}

RegexAST* RegexParser::parse_group_element() {
	if (RegexAST* node = this->parse_group_range()) {
		return node;
	} else if (int32_t* x = this->parse_group_literal()) {
		RegexAST* node = new RegexASTLiteral(*x);
		delete x;
		return node;
	}
	return NULL;
}

int32_t* RegexParser::parse_group_literal() {
	int32_t ch = this->buffer_char();
	if (ch == 0) {
		return NULL;
	}
	if (ch == '\\') {
		ch = this->buffer_char(1);
		if (ch == '-' || ch == ']') {
			this->buffer_advance(2);
			return new int32_t(ch);
		}
	}
	if (ch == '-' || ch == ']') {
		return NULL;
	}
	this->buffer_advance(1);
	return new int32_t(ch);
}

RegexAST* RegexParser::parse_group_range() {
	int32_t* lower = this->parse_group_literal();
	if (!lower) {
		return NULL;
	}
	if (this->buffer_char() != '-') {
		delete lower;
		this->buffer_pop(1);
		return NULL;
	}
	this->buffer_advance(1);
	int32_t* upper = this->parse_group_literal();
	if (!upper) {
		delete lower;
		this->buffer_pop(2);
		return NULL;
	}
	RegexAST* node = new RegexASTRange(*lower, *upper);
	delete lower;
	delete upper;
	this->buffer_push(this->buffer_pop(3));
	return node;
}

int32_t RegexParser::buffer_pos() {
	return this->pos.top();
}

void RegexParser::buffer_advance(int32_t delta) {
	this->pos.push(this->pos.top() + delta);
}

int32_t RegexParser::buffer_char(int32_t delta) {
	if (this->pos.top() + delta >= this->buffer.size()) {
		return 0;
	}
	return this->buffer[this->pos.top() + delta];
}

void RegexParser::buffer_push(int32_t loc) {
	this->pos.push(loc);
}

int32_t RegexParser::buffer_pop(int32_t times) {
	int32_t popped = this->pos.top();
	for (int32_t i = 0; i < times; i++) {
		this->pos.pop();
	}
	return popped;
}

bool RegexParser::is_special_char(int32_t ch) {
	return ch == '\\'
		|| ch == '['
		|| ch == ']'
		|| ch == '('
		|| ch == ')'
		|| ch == '{'
		|| ch == '}'
		|| ch == '|'
		|| ch == '*'
		|| ch == '+'
		|| ch == '?';
}
