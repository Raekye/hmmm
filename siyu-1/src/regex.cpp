#include "regex.h"
#include <cctype>
#include <iterator>

#pragma mark - RegexAST
RegexASTConcat::RegexASTConcat(RegexAST* curr, RegexAST* next) : curr(curr), next(next) {
	return;
}

RegexASTLiteral::RegexASTLiteral(UInt ch) : ch(ch) {
	return;
}

RegexASTOr::RegexASTOr(RegexAST* left, RegexAST* right) : left(left), right(right) {
	return;
}

RegexASTStar::RegexASTStar(RegexAST* node) : node(node) {
	return;
}

RegexASTPhony::RegexASTPhony(RegexAST* node) : node(node) {
	return;
}

RegexAST::~RegexAST() {
	return;
}

RegexASTLiteral::~RegexASTLiteral() {
	return;
}

RegexASTConcat::~RegexASTConcat() {
	delete this->curr;
	delete this->next;
}

RegexASTOr::~RegexASTOr() {
	delete this->left;
	delete this->right;
}

RegexASTStar::~RegexASTStar() {
	delete this->node;
}

void RegexASTEpsilon::accept(IRegexASTVisitor* visitor) {
	visitor->visit(this);
}

void RegexASTLiteral::accept(IRegexASTVisitor* visitor) {
	visitor->visit(this);
}

void RegexASTConcat::accept(IRegexASTVisitor* visitor) {
	visitor->visit(this);
}

void RegexASTOr::accept(IRegexASTVisitor* visitor) {
	visitor->visit(this);
}

void RegexASTStar::accept(IRegexASTVisitor* visitor) {
	visitor->visit(this);
}

void RegexASTPhony::accept(IRegexASTVisitor* visitor) {
	visitor->visit(this);
}

#pragma mark - RegexAST - dfa state generation
bool RegexASTEpsilon::nullable() {
	return true;
}

std::vector<RegexAST*> RegexASTEpsilon::first_pos() {
	std::vector<RegexAST*> vec;
	return vec;
}

std::vector<RegexAST*> RegexASTEpsilon::last_pos() {
	std::vector<RegexAST*> vec;
	return vec;
}

std::vector<RegexAST*> RegexASTEpsilon::follow_pos() {
	std::vector<RegexAST*> vec;
	return vec;
}

bool RegexASTLiteral::nullable() {
	return false;
}

std::vector<RegexAST*> RegexASTLiteral::first_pos() {
	std::vector<RegexAST*> vec;
	vec.push_back(this);
	return vec;
}

std::vector<RegexAST*> RegexASTLiteral::last_pos() {
	std::vector<RegexAST*> vec;
	return vec;
}

std::vector<RegexAST*> RegexASTLiteral::follow_pos() {
	std::vector<RegexAST*> vec;
	return vec;
}

bool RegexASTConcat::nullable() {
	return this->curr->nullable() && this->next->nullable();
}

std::vector<RegexAST*> RegexASTConcat::first_pos() {
	std::vector<RegexAST*> vec = this->curr->first_pos();
	if (this->curr->nullable()) {
		std::vector<RegexAST*> vec2 = this->next->first_pos();
		vec.insert(vec.end(), vec2.begin(), vec2.end());
	}
	return vec;
}

std::vector<RegexAST*> RegexASTConcat::last_pos() {
	std::vector<RegexAST*> vec;
	return vec;
}

std::vector<RegexAST*> RegexASTConcat::follow_pos() {
	std::vector<RegexAST*> vec;
	return vec;
}

bool RegexASTOr::nullable() {
	return this->left->nullable() || this->right->nullable();
}

std::vector<RegexAST*> RegexASTOr::first_pos() {
	std::vector<RegexAST*> vec = this->left->first_pos();
	std::vector<RegexAST*> vec2 = this->right->first_pos();
	vec.insert(vec.end(), vec2.begin(), vec2.end());
	return vec;
}

std::vector<RegexAST*> RegexASTOr::last_pos() {
	std::vector<RegexAST*> vec;
	return vec;
}

std::vector<RegexAST*> RegexASTOr::follow_pos() {
	std::vector<RegexAST*> vec;
	return vec;
}

bool RegexASTStar::nullable() {
	return true;
}

std::vector<RegexAST*> RegexASTStar::first_pos() {
	return this->node->first_pos();
}

std::vector<RegexAST*> RegexASTStar::last_pos() {
	std::vector<RegexAST*> vec;
	return vec;
}

std::vector<RegexAST*> RegexASTStar::follow_pos() {
	std::vector<RegexAST*> vec;
	return vec;
}

bool RegexASTPhony::nullable() {
	return this->node->nullable();
}

std::vector<RegexAST*> RegexASTPhony::first_pos() {
	return this->node->first_pos();
}

std::vector<RegexAST*> RegexASTPhony::last_pos() {
	return this->node->last_pos();
}

std::vector<RegexAST*> RegexASTPhony::follow_pos() {
	return this->node->follow_pos();
}

#pragma mark - RegexParser
Int RegexParser::buffer_pos() {
	return this->pos.top();
}

void RegexParser::buffer_advance(Int delta) {
	this->pos.push(this->buffer_pos() + delta);
}

UInt RegexParser::buffer_char(Int delta) {
	if (this->buffer_pos() + delta >= this->buffer.size()) {
		return 0;
	}
	return this->buffer[this->buffer_pos() + delta];
}

void RegexParser::buffer_push(Int loc) {
	this->pos.push(loc);
}

Int RegexParser::buffer_pop(Int times) {
	Int popped = this->buffer_pos();
	for (Int i = 0; i < times; i++) {
		this->pos.pop();
	}
	return popped;
}

#pragma mark - RegexParser - make regex ast
RegexAST* RegexParser::make_regex_ast_literal(UInt ch) {
	return new RegexASTLiteral(ch);
}

RegexAST* RegexParser::make_regex_ast_add(std::vector<RegexAST*>* chain) {
	RegexAST* node = chain->back();
	for (std::vector<RegexAST*>::reverse_iterator it = chain->rbegin() + 1; it != chain->rend(); it++) {
		node = new RegexASTConcat(*it, node);
	}
	return node;
}

RegexAST* RegexParser::make_regex_ast_or(RegexAST* l, RegexAST* r) {
	return new RegexASTOr(l, r);
}

RegexAST* RegexParser::make_regex_ast_mul(RegexAST* node, UInt min, UInt max) {
	RegexAST* required_nodes = nullptr;
	if (min == 0) {
		return new RegexASTStar(node);
	} else {
		required_nodes = node;
		for (UInt i = 0; i < min; i++) {
			required_nodes = new RegexASTConcat(new RegexASTPhony(node), required_nodes);
		}
	}
	RegexAST* optional_nodes = nullptr;
	if (max == 0) {
		optional_nodes = new RegexASTEpsilon();
	} else {
		RegexAST* optional_repetition = new RegexASTPhony(new RegexASTOr(new RegexASTPhony(node), new RegexASTEpsilon()));
		optional_nodes = optional_repetition;
		for (UInt i = min; i < max; i++) {
			optional_nodes = new RegexASTConcat(optional_repetition, optional_nodes);
		}
	}
	return new RegexASTConcat(required_nodes, optional_nodes);
}

RegexAST* RegexParser::make_regex_ast_range(UInt lower, UInt upper) {
	RegexAST* node = new RegexASTLiteral(upper);
	for (UInt i = upper - 1; i >= lower; i--) {
		node = new RegexASTOr(RegexParser::make_regex_ast_literal(i), node);
	}
	return node;
}

#pragma mark - RegexParser - parsing
RegexAST* RegexParser::parse(std::string str) {
	this->buffer = str;
	this->pos = std::stack<Int>();
	this->pos.push(0);
	RegexAST* regex = this->parse_toplevel();
	if (!regex) {
		return nullptr;
	}
	if (this->buffer_pos() != str.length()) {
		delete regex;
		return nullptr;
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
		return nullptr;
	}
	if (this->buffer_char() != RegexParser::TOKEN_OR) {
		return l;
	}
	this->buffer_advance(1);
	RegexAST* r = this->parse_lr_or();
	if (!r) {
		delete l;
		this->buffer_pop(2);
		return nullptr;
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
		return nullptr;
	}
	std::vector<RegexAST*> chain;
	chain.push_back(car);
	while (true) {
		RegexAST* next = this->parse_not_lr_add();
		if (!next) {
			break;
		}
		chain.push_back(next);
		this->buffer_push(this->buffer_pop(2));
	}
	return RegexParser::make_regex_ast_add(&chain);
}

RegexAST* RegexParser::parse_not_lr_add() {
	return this->parse_lr_mul();
}

RegexAST* RegexParser::parse_lr_mul() {
	RegexAST* l = this->parse_not_lr_mul();
	UInt ch = this->buffer_char();
	if (ch == RegexParser::TOKEN_STAR) {
		this->buffer_advance(1);
		this->buffer_push(this->buffer_pop(2));
		return RegexParser::make_regex_ast_mul(l, 0, 0);
	} else if (ch == RegexParser::TOKEN_PLUS) {
		this->buffer_advance(1);
		this->buffer_push(this->buffer_pop(2));
		return RegexParser::make_regex_ast_mul(l, 1, 0);
	} else if (ch == RegexParser::TOKEN_QUESTION_MARK) {
		this->buffer_advance(1);
		this->buffer_push(this->buffer_pop(2));
		return RegexParser::make_regex_ast_mul(l, 0, 1);
	}
	std::tuple<UInt, UInt>* range = this->parse_mul_range();
	if (!range) {
		return l;
	}
	this->buffer_push(this->buffer_pop(2));
	RegexAST* r = RegexParser::make_regex_ast_mul(l, std::get<0>(*range), std::get<1>(*range));
	delete range;
	return r;
}

std::tuple<UInt, UInt>* RegexParser::parse_mul_range() {
	if (this->buffer_char() != '{') {
		return nullptr;
	}
	this->buffer_advance(1);
	UInt* lower = this->parse_dec_int();
	if (!lower) {
		this->buffer_pop(1);
		return nullptr;
	}
	if (this->buffer_char() != ',') {
		this->buffer_pop(2);
		return nullptr;
	}
	this->buffer_advance(1);
	UInt* upper = this->parse_dec_int();
	if (!upper) {
		delete lower;
		this->buffer_pop(3);
		return nullptr;
	}
	if (this->buffer_char() != '}') {
		delete lower;
		delete upper;
		this->buffer_pop(4);
		return nullptr;
	}
	this->buffer_push(this->buffer_pop(4) + 1);
	std::tuple<UInt, UInt>* range = new std::tuple<UInt, UInt>(*lower, *upper);
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
	return nullptr;
}

RegexAST* RegexParser::parse_parentheses() {
	if (this->buffer_char() != '(') {
		return nullptr;
	}
	this->buffer_advance(1);
	RegexAST* node = this->parse_toplevel();
	if (!node) {
		this->buffer_pop(1);
		return nullptr;
	}
	if (this->buffer_char() != ')') {
		delete node;
		this->buffer_pop(2);
		return nullptr;
	}
	this->buffer_push(this->buffer_pop(2) + 1);
	return node;
}

RegexAST* RegexParser::parse_literal() {
	UInt* x = this->parse_absolute_literal();
	if (x) {
		RegexAST* r = RegexParser::make_regex_ast_literal(*x);
		delete x;
		return r;
	}
	Int ch = this->buffer_char();
	if (ch == RegexParser::TOKEN_ESCAPE) {
		ch = this->buffer_char(1);
		if (RegexParser::is_special_char(ch)) {
			this->buffer_advance(2);
			return RegexParser::make_regex_ast_literal(ch);
		}
		return nullptr;
	}
	if (!RegexParser::is_special_char(ch) && 32 <= ch && ch < 127) {
		this->buffer_advance(1);
		return RegexParser::make_regex_ast_literal(ch);
	}
	return nullptr;
}

RegexAST* RegexParser::parse_group() {
	if (this->buffer_char() != RegexParser::TOKEN_LBRACKET) {
		return nullptr;
	}
	this->buffer_advance(1);
	RegexAST* contents = this->parse_group_contents();
	if (!contents) {
		this->buffer_pop(1);
		return nullptr;
	}
	if (this->buffer_char() != RegexParser::TOKEN_RBRACKET) {
		delete contents;
		this->buffer_pop(2);
		return nullptr;
	}
	this->buffer_advance(1);
	this->buffer_push(this->buffer_pop(3));
	return contents;
}

RegexAST* RegexParser::parse_group_contents() {
	RegexAST* car = this->parse_group_element();
	if (!car) {
		return nullptr;
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
	} else if (UInt* x= this->parse_group_literal()) {
		 RegexAST* r = RegexParser::make_regex_ast_literal(*x);
		 delete x;
		 return r;
	}
	return nullptr;
}

UInt* RegexParser::parse_group_literal() {
	UInt* l = this->parse_absolute_literal();
	if (l) {
		return l;
	}
	Int ch = this->buffer_char();
	if (ch == RegexParser::TOKEN_ESCAPE) {
		ch = this->buffer_char(1);
		if (RegexParser::is_group_special_char(ch)) {
			this->buffer_advance(2);
			return new UInt(ch);
		}
		return nullptr;
	}
	if (!RegexParser::is_group_special_char(ch) && 32 <= ch && ch < 127) {
		this->buffer_advance(1);
		return new UInt(ch);
	}
	return nullptr;
}

RegexAST* RegexParser::parse_group_range() {
	UInt* lower = this->parse_group_literal();
	if (!lower) {
		return nullptr;
	}
	if (this->buffer_char() != '-') {
		delete lower;
		this->buffer_pop(1);
		return nullptr;
	}
	this->buffer_advance(1);
	UInt* upper = this->parse_group_literal();
	if (!upper) {
		delete lower;
		this->buffer_pop(2);
		return nullptr;
	}
	RegexAST* node = RegexParser::make_regex_ast_range(*lower, *upper);
	delete lower;
	delete upper;
	this->buffer_push(this->buffer_pop(3));
	return node;
}

UInt* RegexParser::parse_absolute_literal() {
	if (this->buffer_char() != RegexParser::TOKEN_ESCAPE) {
		return nullptr;
	}
	this->buffer_advance(1);
	UInt ch = this->buffer_char();
	if (ch == RegexParser::TOKEN_X) {
		this->buffer_advance(1);
		UInt* x = this->parse_hex_byte();
		if (!x) {
			this->buffer_pop(2);
			return nullptr;
		}
		this->buffer_push(this->buffer_pop(2));
		return x;
	} else if (ch == RegexParser::TOKEN_U) {
		this->buffer_advance(1);
		UInt* x = this->parse_hex_int();
		if (!x) {
			this->buffer_pop(2);
			return nullptr;
		}
		this->buffer_push(this->buffer_pop(2));
		return x;
	} else if (ch == RegexParser::TOKEN_T) {
		this->buffer_advance(1);
		this->buffer_push(this->buffer_pop(2));
		return new UInt('\t');
	} else if (ch == RegexParser::TOKEN_N) {
		this->buffer_advance(1);
		this->buffer_push(this->buffer_pop(2));
		return new UInt('\n');
	} else if (ch == RegexParser::TOKEN_R) {
		this->buffer_advance(1);
		this->buffer_push(this->buffer_pop(2));
		return new UInt('\r');
	}
	this->buffer_pop(1);
	return nullptr;
}

UInt* RegexParser::parse_hex_byte() {
	Int upper = this->buffer_char();
	if (!RegexParser::is_hex_digit(upper)) {
		return nullptr;
	}
	Int lower = this->buffer_char(1);
	if (!RegexParser::is_hex_digit(lower)) {
		return nullptr;
	}
	UInt x = 0;
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
	return new UInt(x);
}

UInt* RegexParser::parse_hex_int() {
	UInt x = 0;
	this->buffer_advance(0);
	for (Int i = 0; i < 4; i++) {
		UInt* b = this->parse_hex_byte();
		if (!b) {
			this->buffer_pop(1);
			return nullptr;
		}
		x = (x << 8) + *b;
		delete b;
		this->buffer_push(this->buffer_pop(2));
	}
	return new UInt(x);
}

UInt* RegexParser::parse_dec_int() {
	UInt x = 0;
	Int delta = 0;
	while (true) {
		Int ch = this->buffer_char(delta);
		if (!RegexParser::is_dec_digit(ch)) {
			break;
		}
		x = x * 10 + (ch - '0');
		delta++;
	}
	this->buffer_advance(delta);
	return new UInt(x);
}

bool RegexParser::is_special_char(UInt ch) {
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

bool RegexParser::is_group_special_char(UInt ch) {
	return ch == TOKEN_LBRACKET
		|| ch == TOKEN_DASH
		|| ch == TOKEN_RBRACKET
		|| ch == TOKEN_ESCAPE;
}

bool RegexParser::is_hex_digit(UInt ch) {
	return ('0' <= ch && ch <= '9') || ('a' <= ch && ch <= 'f');
}

bool RegexParser::is_dec_digit(UInt ch) {
	return ('0' <= ch && ch <= '9');
}
