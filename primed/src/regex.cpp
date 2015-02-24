#include "regex.h"
#include <cctype>
#include <iostream>

#pragma mark - RegexAST

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

RegexASTRange::RegexASTRange(uint32_t lower, uint32_t upper) {
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

#pragma mark - RegexParser
RegexParser::RegexParser() {
	return;
}

int32_t RegexParser::buffer_pos() {
	return this->pos.top();
}

void RegexParser::buffer_advance(int32_t delta) {
	this->pos.push(this->buffer_pos() + delta);
}

uint32_t RegexParser::buffer_char(int32_t delta) {
	if (this->buffer_pos() + delta >= this->buffer.size()) {
		return 0;
	}
	return this->buffer[this->buffer_pos() + delta];
}

void RegexParser::buffer_push(int32_t loc) {
	this->pos.push(loc);
}

int32_t RegexParser::buffer_pop(int32_t times) {
	int32_t popped = this->buffer_pos();
	for (int32_t i = 0; i < times; i++) {
		this->pos.pop();
	}
	return popped;
}

#pragma mark - RegexParser - parsing
RegexAST* RegexParser::parse(std::string str) {
	this->buffer = str;
	this->pos = std::stack<int32_t>();
	this->pos.push(0);
	RegexAST* regex = this->parse_toplevel();
	if (!regex) {
		return NULL;
	}
	if (this->buffer_pos() != str.length()) {
		delete regex;
		return NULL;
	}
	this->pos.pop();
	if (this->pos.size() != 1) {
		delete regex;
		throw std::runtime_error("RegexParser did not finish with pos stack 1");
	}
	return regex;
}

RegexAST* RegexParser::parse_toplevel() {
	return this->parse_lr_or();
}

RegexAST* RegexParser::parse_lr_or() {
	RegexAST* l = this->parse_not_lr_or();
	if (!l) {
		return NULL;
	}
	if (this->buffer_char() != RegexParser::TOKEN_OR) {
		return l;
	}
	this->buffer_advance(1);
	RegexAST* r = this->parse_lr_or();
	if (!r) {
		delete l;
		this->buffer_pop(2);
		return NULL;
	}
	this->buffer_push(this->buffer_pop(3));
	return new RegexASTOr(l, r);
}

RegexAST* RegexParser::parse_not_lr_or() {
	return this->parse_lr_add();
}

RegexAST* RegexParser::parse_lr_add() {
	RegexAST* car = this->parse_not_lr_add();
	if (!car) {
		return NULL;
	}
	std::vector<RegexAST*>* chain = new std::vector<RegexAST*>();
	chain->push_back(car);
	while (true) {
		RegexAST* next = this->parse_not_lr_add();
		if (!next) {
			break;
		}
		chain->push_back(next);
		this->buffer_push(this->buffer_pop(2));
	}
	return new RegexASTChain(chain);
}

RegexAST* RegexParser::parse_not_lr_add() {
	return this->parse_lr_mul();
}

RegexAST* RegexParser::parse_lr_mul() {
	RegexAST* l = this->parse_not_lr_mul();
	uint32_t ch = this->buffer_char();
	if (ch == RegexParser::TOKEN_STAR) {
		this->buffer_advance(1);
		this->buffer_push(this->buffer_pop(2));
		return new RegexASTMultiplication(l, 0, 0);
	} else if (ch == RegexParser::TOKEN_PLUS) {
		this->buffer_advance(1);
		this->buffer_push(this->buffer_pop(2));
		return new RegexASTMultiplication(l, 1, 0);
	} else if (ch == RegexParser::TOKEN_QUESTION_MARK) {
		this->buffer_advance(1);
		this->buffer_push(this->buffer_pop(2));
		return new RegexASTMultiplication(l, 0, 1);
	}
	std::tuple<int32_t, int32_t>* range = this->parse_mul_range();
	if (!range) {
		return l;
	}
	this->buffer_push(this->buffer_pop(2));
	RegexAST* r = new RegexASTMultiplication(l, std::get<0>(*range), std::get<1>(*range));
	delete range;
	return r;
}

std::tuple<int32_t, int32_t>* RegexParser::parse_mul_range() {
	if (this->buffer_char() != '{') {
		return NULL;
	}
	this->buffer_advance(1);
	uint32_t* lower = this->parse_dec_int();
	if (!lower) {
		this->buffer_pop(1);
		return NULL;
	}
	if (this->buffer_char() != ',') {
		this->buffer_pop(2);
		return NULL;
	}
	this->buffer_advance(1);
	uint32_t* upper = this->parse_dec_int();
	if (!upper) {
		delete lower;
		this->buffer_pop(3);
		return NULL;
	}
	if (this->buffer_char() != '}') {
		delete lower;
		delete upper;
		this->buffer_pop(4);
		return NULL;
	}
	this->buffer_push(this->buffer_pop(4) + 1);
	std::tuple<int32_t, int32_t>* range = new std::tuple<int32_t, int32_t>(*lower, *upper);
	delete lower;
	delete upper;
	return range;
}

RegexAST* RegexParser::parse_not_lr_mul() {
	return this->parse_not_lr();
}

RegexAST* RegexParser::parse_not_lr() {
	if (RegexAST* r = this->parse_parentheses()) {
		return r;
	} else if (RegexAST* r = this->parse_literal()) {
		return r;
	} else if (RegexAST* r = this->parse_group()) {
		return r;
	}
	return NULL;
}

RegexAST* RegexParser::parse_parentheses() {
	if (this->buffer_char() != '(') {
		return NULL;
	}
	this->buffer_advance(1);
	RegexAST* node = this->parse_toplevel();
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
	uint32_t* x = this->parse_absolute_literal();
	if (x) {
		RegexAST* r = new RegexASTLiteral(*x);
		delete x;
		return r;
	}
	int32_t ch = this->buffer_char();
	if (ch == RegexParser::TOKEN_ESCAPE) {
		ch = this->buffer_char(1);
		if (RegexParser::is_special_char(ch)) {
			this->buffer_advance(2);
			return new RegexASTLiteral(ch);
		}
		return NULL;
	}
	if (!RegexParser::is_special_char(ch) && 32 <= ch && ch < 127) {
		this->buffer_advance(1);
		return new RegexASTLiteral(ch);
	}
	return NULL;
}

RegexAST* RegexParser::parse_group() {
	if (this->buffer_char() != RegexParser::TOKEN_LBRACKET) {
		return NULL;
	}
	this->buffer_advance(1);
	RegexAST* contents = this->parse_group_contents();
	if (!contents) {
		this->buffer_pop(1);
		return NULL;
	}
	if (this->buffer_char() != RegexParser::TOKEN_RBRACKET) {
		delete contents;
		this->buffer_pop(2);
		return NULL;
	}
	this->buffer_advance(1);
	this->buffer_push(this->buffer_pop(3));
	return contents;
}

RegexAST* RegexParser::parse_group_contents() {
	RegexAST* car = this->parse_group_element();
	if (!car) {
		return NULL;
	}
	RegexAST* cdr = this->parse_group_contents();
	if (!cdr) {
		return car;
	}
	this->buffer_push(this->buffer_pop(2));
	return new RegexASTOr(car, cdr);
}

RegexAST* RegexParser::parse_group_element() {
	if (RegexAST* node = this->parse_group_range()) {
		return node;
	} else if (uint32_t* x= this->parse_group_literal()) {
		 RegexAST* r =new RegexASTLiteral(*x);
		 delete x;
		 return r;
	}
	return NULL;
}

uint32_t* RegexParser::parse_group_literal() {
	uint32_t* l = this->parse_absolute_literal();
	if (l) {
		return l;
	}
	int32_t ch = this->buffer_char();
	if (ch == RegexParser::TOKEN_ESCAPE) {
		ch = this->buffer_char(1);
		if (RegexParser::is_group_special_char(ch)) {
			this->buffer_advance(2);
			return new uint32_t(ch);
		}
		return NULL;
	}
	if (!RegexParser::is_group_special_char(ch) && 32 <= ch && ch < 127) {
		this->buffer_advance(1);
		return new uint32_t(ch);
	}
	return NULL;
}

RegexAST* RegexParser::parse_group_range() {
	uint32_t* lower = this->parse_group_literal();
	if (!lower) {
		return NULL;
	}
	if (this->buffer_char() != '-') {
		delete lower;
		this->buffer_pop(1);
		return NULL;
	}
	this->buffer_advance(1);
	uint32_t* upper = this->parse_group_literal();
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

uint32_t* RegexParser::parse_absolute_literal() {
	if (this->buffer_char() != RegexParser::TOKEN_ESCAPE) {
		return NULL;
	}
	this->buffer_advance(1);
	uint32_t ch = this->buffer_char();
	if (ch == RegexParser::TOKEN_X) {
		this->buffer_advance(1);
		uint32_t* x = this->parse_hex_byte();
		if (!x) {
			this->buffer_pop(2);
			return NULL;
		}
		this->buffer_push(this->buffer_pop(2));
		return x;
	} else if (ch == RegexParser::TOKEN_U) {
		this->buffer_advance(1);
		uint32_t* x = this->parse_hex_int();
		if (!x) {
			this->buffer_pop(2);
			return NULL;
		}
		this->buffer_push(this->buffer_pop(2));
		return x;
	} else if (ch == RegexParser::TOKEN_T) {
		this->buffer_advance(1);
		this->buffer_push(this->buffer_pop(2));
		return new uint32_t('\t');
	} else if (ch == RegexParser::TOKEN_N) {
		this->buffer_advance(1);
		this->buffer_push(this->buffer_pop(2));
		return new uint32_t('\n');
	} else if (ch == RegexParser::TOKEN_R) {
		this->buffer_advance(1);
		this->buffer_push(this->buffer_pop(2));
		return new uint32_t('\r');
	}
	this->buffer_pop(1);
	return NULL;
}

uint32_t* RegexParser::parse_hex_byte() {
	int32_t upper = this->buffer_char();
	if (!RegexParser::is_hex_digit(upper)) {
		return NULL;
	}
	int32_t lower = this->buffer_char(1);
	if (!RegexParser::is_hex_digit(lower)) {
		return NULL;
	}
	uint32_t x = 0;
	if (RegexParser::is_dec_digit(upper)) {
		x = upper - '0';
	} else {
		x = upper - 'a' + 10;
	}
	x <<= 4;
	if (RegexParser::is_dec_digit(lower)) {
		x |= upper - '0';
	} else {
		x |= upper - 'a' + 10;
	}
	this->buffer_advance(2);
	return new uint32_t(x);
}

uint32_t* RegexParser::parse_hex_int() {
	uint32_t x = 0;
	this->buffer_advance(0);
	for (int32_t i = 0; i < 4; i++) {
		uint32_t* b = this->parse_hex_byte();
		if (!b) {
			this->buffer_pop(1);
			return NULL;
		}
		x = (x << 8) + *b;
		delete b;
		this->buffer_push(this->buffer_pop(2));
	}
	return new uint32_t(x);
}

uint32_t* RegexParser::parse_dec_int() {
	uint32_t x = 0;
	int32_t delta = 0;
	while (true) {
		int32_t ch = this->buffer_char(delta);
		if (!RegexParser::is_dec_digit(ch)) {
			break;
		}
		x = x * 10 + (ch - '0');
		delta++;
	}
	this->buffer_advance(delta);
	return new uint32_t(x);
}

bool RegexParser::is_special_char(uint32_t ch) {
	return ch == TOKEN_ESCAPE
		|| ch == TOKEN_LPAREN
		|| ch == TOKEN_RPAREN
		|| ch == TOKEN_LBRACE
		|| ch == TOKEN_RBRACE
		|| ch == TOKEN_LBRACKET
		|| ch == TOKEN_RBRACKET
		|| ch == TOKEN_OR
		|| ch == TOKEN_STAR
		|| ch == TOKEN_PLUS
		|| ch == TOKEN_QUESTION_MARK;
}

bool RegexParser::is_group_special_char(uint32_t ch) {
	return ch == TOKEN_LBRACKET
		|| ch == TOKEN_DASH
		|| ch == TOKEN_RBRACKET
		|| ch == TOKEN_ESCAPE;
}

bool RegexParser::is_hex_digit(uint32_t ch) {
	return ('0' <= ch && ch <= '9') || ('a' <= ch && ch <= 'f');
}

bool RegexParser::is_dec_digit(uint32_t ch) {
	return ('0' <= ch && ch <= '9');
}

#pragma mark - RegexDFAGenerator

void RegexDFAGenerator::visit(RegexASTChain* node) {
	DFAState<uint32_t>* saved_root = this->root;
	node->sequence->front()->accept(this);
	this->root = this->ret;
	for (int32_t i = 1; i < node->sequence->size(); i++) {
		node->sequence->operator[](i)->accept(this);
		this->root = this->ret;
	}
	this->root = saved_root;
	// ret keeps last value
}

void RegexDFAGenerator::visit(RegexASTLiteral* node) {
	std::map<uint32_t, DFAState<uint32_t>*>::iterator it = this->root->link->find(node->ch);
	if (it == this->root->link->end()) {
		this->ret = new DFAState<uint32_t>();
	} else {
		this->ret = it->second;
	}
}

void RegexDFAGenerator::visit(RegexASTOr* node) {
	node->left->accept(this);
	DFAState<uint32_t>* saved_left_ret = this->ret;
	node->right->accept(this);
	for (std::map<uint32_t, DFAState<uint32_t>*>::iterator it = saved_left_ret->link->begin(); it != saved_left_ret->link->end(); it++) {
		std::map<uint32_t, DFAState<uint32_t>*>::iterator it2 = this->ret->link->find(it->first);
		if (it2 == this->ret->link->end()) {
			this->ret->link->operator[](it->first) = it->second;
		} else {
			if (it2->second != it->second) {
				throw std::runtime_error("State badness");
			}
		}
	}
	delete saved_left_ret->link;
	saved_left_ret->link = this->ret->link;
	// root unchanged
	// ret keeps last generated value
}

void RegexDFAGenerator::visit(RegexASTMultiplication* node) {
	return;
}

void RegexDFAGenerator::visit(RegexASTRange* node) {
	return;
}
