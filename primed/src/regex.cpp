#include "regex.h"
#include <cctype>
#include <iterator>

#pragma mark - RegexAST
RegexASTChain::RegexASTChain(std::vector<RegexAST*> sequence) : sequence(sequence) {
	return;
}

RegexASTLiteral::RegexASTLiteral(Int ch) {
	this->ch = ch;
}

RegexASTOr::RegexASTOr(RegexAST* left, RegexAST* right) {
	this->left = left;
	this->right = right;
}

RegexASTMultiplication::RegexASTMultiplication(RegexAST* node, Int min, Int max) {
	this->node = node;
	this->min = min;
	this->max = max;
}

RegexASTRange::RegexASTRange(UInt lower, UInt upper) : lower(lower), upper(upper) {
	for (UInt i = lower; i <= upper; i++) {
		this->nodes.push_back(new RegexASTLiteral(i));
	}
}

RegexAST::~RegexAST() {
	return;
}

RegexASTChain::~RegexASTChain() {
	for (Int i = 0; i < this->sequence.size(); i++) {
		delete this->sequence[i];
	}
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
	for (std::vector<RegexAST*>::iterator it = this->nodes.begin(); it != this->nodes.end(); it++) {
		delete *it;
	}
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

bool RegexASTMultiplication::is_infinite() {
	return this->max == 0;
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

#pragma mark - RegexParser - parsing
RegexAST* RegexParser::parse(std::string str) {
	this->buffer = str;
	this->pos = std::stack<Int>();
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
	return new RegexASTChain(chain);
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
	std::tuple<UInt, UInt>* range = this->parse_mul_range();
	if (!range) {
		return l;
	}
	this->buffer_push(this->buffer_pop(2));
	RegexAST* r = new RegexASTMultiplication(l, std::get<0>(*range), std::get<1>(*range));
	delete range;
	return r;
}

std::tuple<UInt, UInt>* RegexParser::parse_mul_range() {
	if (this->buffer_char() != '{') {
		return NULL;
	}
	this->buffer_advance(1);
	UInt* lower = this->parse_dec_int();
	if (!lower) {
		this->buffer_pop(1);
		return NULL;
	}
	if (this->buffer_char() != ',') {
		this->buffer_pop(2);
		return NULL;
	}
	this->buffer_advance(1);
	UInt* upper = this->parse_dec_int();
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
	UInt* x = this->parse_absolute_literal();
	if (x) {
		RegexAST* r = new RegexASTLiteral(*x);
		delete x;
		return r;
	}
	Int ch = this->buffer_char();
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
	} else if (UInt* x= this->parse_group_literal()) {
		 RegexAST* r =new RegexASTLiteral(*x);
		 delete x;
		 return r;
	}
	return NULL;
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
		return NULL;
	}
	if (!RegexParser::is_group_special_char(ch) && 32 <= ch && ch < 127) {
		this->buffer_advance(1);
		return new UInt(ch);
	}
	return NULL;
}

RegexAST* RegexParser::parse_group_range() {
	UInt* lower = this->parse_group_literal();
	if (!lower) {
		return NULL;
	}
	if (this->buffer_char() != '-') {
		delete lower;
		this->buffer_pop(1);
		return NULL;
	}
	this->buffer_advance(1);
	UInt* upper = this->parse_group_literal();
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

UInt* RegexParser::parse_absolute_literal() {
	if (this->buffer_char() != RegexParser::TOKEN_ESCAPE) {
		return NULL;
	}
	this->buffer_advance(1);
	UInt ch = this->buffer_char();
	if (ch == RegexParser::TOKEN_X) {
		this->buffer_advance(1);
		UInt* x = this->parse_hex_byte();
		if (!x) {
			this->buffer_pop(2);
			return NULL;
		}
		this->buffer_push(this->buffer_pop(2));
		return x;
	} else if (ch == RegexParser::TOKEN_U) {
		this->buffer_advance(1);
		UInt* x = this->parse_hex_int();
		if (!x) {
			this->buffer_pop(2);
			return NULL;
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
	return NULL;
}

UInt* RegexParser::parse_hex_byte() {
	Int upper = this->buffer_char();
	if (!RegexParser::is_hex_digit(upper)) {
		return NULL;
	}
	Int lower = this->buffer_char(1);
	if (!RegexParser::is_hex_digit(lower)) {
		return NULL;
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
			return NULL;
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

#pragma mark - RegexNFAGenerator
/*
 * # Implementation details
 * RegexAST supports the visitor pattern so other classes can traverse it too.
 * To simulate passing arguments, there are instance variables `root`, and `target_state`
 * - `root`: parent or current state, depending on how you look at it
 * - `target_state`: the node should end at this state
 * Arguments should be restored before returning
 */
RegexNFAGenerator::RegexNFAGenerator() {
	this->reset();
}

void RegexNFAGenerator::reset() {
	this->root = this->nfa.root;
	this->target_state = this->nfa.new_state();
	this->target_state->terminal = true;
}

RegexNFAState* RegexNFAGenerator::next_state() {
	return this->target_state == NULL ? this->nfa.new_state() : this->target_state;
}

void RegexNFAGenerator::visit(RegexASTChain* node) {
	RegexNFAState* saved_root = this->root;
	RegexNFAState* saved_target_state = this->target_state;
	UInt i = 1;
	for (auto& each : node->sequence) {
		this->target_state = i == node->sequence.size() ? saved_target_state : this->nfa.new_state();
		each->accept(this);
		this->root = this->target_state;
		i++;
	}
	this->root = saved_root;
}

void RegexNFAGenerator::visit(RegexASTLiteral* node) {
	this->root->next_states[node->ch].push_back(this->target_state);
}

void RegexNFAGenerator::visit(RegexASTOr* node) {
	node->left->accept(this);
	node->right->accept(this);
}

void RegexNFAGenerator::visit(RegexASTMultiplication* node) {
	RegexNFAState* saved_root = this->root;
	RegexNFAState* saved_target_state = this->target_state;

	RegexNFAState* end_state = this->next_state();

	if (node->min == 0) {
		this->root->epsilon = end_state;
	} else {
		for (Int i = 1; i < node->min; i++) {
			this->target_state = this->nfa.new_state();
			node->node->accept(this);
			this->root = this->target_state;
		}
		// last required node, go to end state
		this->target_state = end_state;
		node->node->accept(this);
		// don't set root; may need to generate branch to optional, bounded repetitions
	}

	if (node->is_infinite()) {
		// unbounded loop back
		this->root = end_state;
		this->target_state = end_state;
		node->node->accept(this);
	} else {
		// if `node->min == 0`, `this->root` is already the first optional node
		// if `node->min != 0`, from above, we generated up to the last required node
		// both already have links to the end state, but the last required node needs a link to the first optional node
		if (node->min != 0) {
			// branch to next state
			this->target_state = this->nfa.new_state();
			node->node->accept(this);
			this->root = this->target_state;
		}
		for (Int i = node->min + 1; i < node->max; i++) {
			// branch to end state
			this->target_state = end_state;
			node->node->accept(this);
			// branch to next state
			this->target_state = this->nfa.new_state();
			node->node->accept(this);
			this->root = this->target_state;
		}
		// last possible node, go to end state
		this->target_state = end_state;
		node->node->accept(this);
	}
	this->root = saved_root;
	this->target_state = saved_target_state;
}

void RegexNFAGenerator::visit(RegexASTRange* node) {
	RegexAST* r = new RegexASTLiteral(node->upper);
	for (UInt i = node->upper - 1; i >= node->lower; i--) {
		r = new RegexASTOr(new RegexASTLiteral(i), r);
	}
	r->accept(this);
	delete r;
}
