#include "regex_old.h"
#include <cctype>
#include <iterator>

//namespace mami {
using namespace mami;

#pragma mark - RegexAST
RegexAST::~RegexAST() {
	return;
}

RegexASTLiteral::RegexASTLiteral(UInt ch) : ch(ch) {
	return;
}

RegexASTChain::RegexASTChain(std::vector<std::unique_ptr<RegexAST>> sequence) : sequence(std::move(sequence)) {
	return;
}

RegexASTOr::RegexASTOr(std::unique_ptr<RegexAST> left, std::unique_ptr<RegexAST> right) : left(std::move(left)), right(std::move(right)) {
	return;
}

RegexASTMultiplication::RegexASTMultiplication(std::unique_ptr<RegexAST> node, Int min, Int max) : node(std::move(node)), min(min), max(max) {
	return;
}

RegexASTRange::RegexASTRange(UInt lower, UInt upper) : lower(lower), upper(upper) {
	return;
}

#pragma mark - RegexAT - visitor pattern
void RegexASTLiteral::accept(IRegexASTVisitor* visitor) {
	visitor->visit(this);
}

void RegexASTChain::accept(IRegexASTVisitor* visitor) {
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
	if (this->buffer_pos() + delta >= (Int) this->buffer.size()) {
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

#pragma mark - RegexParser - make RegexAST
std::unique_ptr<RegexAST> RegexParser::make_regex_ast_literal(UInt ch) {
	return std::unique_ptr<RegexAST>(new RegexASTLiteral(ch));
}

std::unique_ptr<RegexAST> RegexParser::make_regex_ast_chain(std::vector<std::unique_ptr<RegexAST>>* vec) {
	return std::unique_ptr<RegexAST>(new RegexASTChain(std::move(*vec)));
}

std::unique_ptr<RegexAST> RegexParser::make_regex_ast_or(std::unique_ptr<RegexAST>* l, std::unique_ptr<RegexAST>* r) {
	return std::unique_ptr<RegexAST>(new RegexASTOr(std::move(*l), std::move(*r)));
}

std::unique_ptr<RegexAST> RegexParser::make_regex_ast_multiplication(std::unique_ptr<RegexAST>* node, UInt min, UInt max) {
	return std::unique_ptr<RegexAST>(new RegexASTMultiplication(std::move(*node), min, max));
}

std::unique_ptr<RegexAST> RegexParser::make_regex_ast_range(UInt lower, UInt upper) {
	return std::unique_ptr<RegexAST>(new RegexASTRange(lower, upper));
}


#pragma mark - RegexParser - parsing
std::unique_ptr<RegexAST> RegexParser::parse(std::string str) {
	this->buffer = str;
	this->pos = std::stack<Int>();
	this->pos.push(0);
	std::unique_ptr<RegexAST> regex = this->parse_toplevel();
	if (!regex) {
		return nullptr;
	}
	if (this->buffer_pos() != (Int) str.length()) {
		return nullptr;
	}
	this->pos.pop();
	if (this->pos.size() != 1) {
		return nullptr;
	}
	return regex;
}

std::unique_ptr<RegexAST> RegexParser::parse_toplevel() {
	return this->parse_lr_or();
}

std::unique_ptr<RegexAST> RegexParser::parse_lr_or() {
	std::unique_ptr<RegexAST> l = this->parse_not_lr_or();
	if (!l) {
		return nullptr;
	}
	if (this->buffer_char() != RegexParser::TOKEN_OR) {
		return l;
	}
	this->buffer_advance(1);
	std::unique_ptr<RegexAST> r = this->parse_lr_or();
	if (!r) {
		this->buffer_pop(2);
		return nullptr;
	}
	this->buffer_push(this->buffer_pop(3));
	return RegexParser::make_regex_ast_or(&l, &r);
}

std::unique_ptr<RegexAST> RegexParser::parse_not_lr_or() {
	return this->parse_lr_add();
}

std::unique_ptr<RegexAST> RegexParser::parse_lr_add() {
	std::unique_ptr<RegexAST> car = this->parse_not_lr_add();
	if (!car) {
		return nullptr;
	}
	std::vector<std::unique_ptr<RegexAST>> chain;
	chain.push_back(std::move(car));
	while (true) {
		std::unique_ptr<RegexAST> next = this->parse_not_lr_add();
		if (!next) {
			break;
		}
		chain.push_back(std::move(next));
		this->buffer_push(this->buffer_pop(2));
	}
	return RegexParser::make_regex_ast_chain(&chain);
}

std::unique_ptr<RegexAST> RegexParser::parse_not_lr_add() {
	return this->parse_lr_mul();
}

std::unique_ptr<RegexAST> RegexParser::parse_lr_mul() {
	std::unique_ptr<RegexAST> l = this->parse_not_lr_mul();
	UInt ch = this->buffer_char();
	if (ch == RegexParser::TOKEN_STAR) {
		this->buffer_advance(1);
		this->buffer_push(this->buffer_pop(2));
		return RegexParser::make_regex_ast_multiplication(&l, 0, 0);
	} else if (ch == RegexParser::TOKEN_PLUS) {
		this->buffer_advance(1);
		this->buffer_push(this->buffer_pop(2));
		return RegexParser::make_regex_ast_multiplication(&l, 1, 0);
	} else if (ch == RegexParser::TOKEN_QUESTION_MARK) {
		this->buffer_advance(1);
		this->buffer_push(this->buffer_pop(2));
		return RegexParser::make_regex_ast_multiplication(&l, 0, 1);
	}
	std::unique_ptr<std::tuple<UInt, UInt>> range = this->parse_mul_range();
	if (!range) {
		return l;
	}
	this->buffer_push(this->buffer_pop(2));
	std::unique_ptr<RegexAST> r = RegexParser::make_regex_ast_multiplication(&l, std::get<0>(*range), std::get<1>(*range));
	return r;
}

std::unique_ptr<std::tuple<UInt, UInt>> RegexParser::parse_mul_range() {
	if (this->buffer_char() != '{') {
		return nullptr;
	}
	this->buffer_advance(1);
	std::unique_ptr<UInt> lower = this->parse_dec_int();
	if (!lower) {
		this->buffer_pop(1);
		return nullptr;
	}
	if (this->buffer_char() != ',') {
		this->buffer_pop(2);
		return nullptr;
	}
	this->buffer_advance(1);
	std::unique_ptr<UInt> upper = this->parse_dec_int();
	if (!upper) {
		this->buffer_pop(3);
		return nullptr;
	}
	if (this->buffer_char() != '}') {
		this->buffer_pop(4);
		return nullptr;
	}
	this->buffer_push(this->buffer_pop(4) + 1);
	std::unique_ptr<std::tuple<UInt, UInt>> range(new std::tuple<UInt, UInt>(*lower, *upper));
	return range;
}

std::unique_ptr<RegexAST> RegexParser::parse_not_lr_mul() {
	return this->parse_not_lr();
}

std::unique_ptr<RegexAST> RegexParser::parse_not_lr() {
	if (std::unique_ptr<RegexAST> r = this->parse_parentheses()) {
		return r;
	} else if (std::unique_ptr<RegexAST> r = this->parse_literal()) {
		return r;
	} else if (std::unique_ptr<RegexAST> r = this->parse_group()) {
		return r;
	}
	return nullptr;
}

std::unique_ptr<RegexAST> RegexParser::parse_parentheses() {
	if (this->buffer_char() != '(') {
		return nullptr;
	}
	this->buffer_advance(1);
	std::unique_ptr<RegexAST> node = this->parse_toplevel();
	if (!node) {
		this->buffer_pop(1);
		return nullptr;
	}
	if (this->buffer_char() != ')') {
		this->buffer_pop(2);
		return nullptr;
	}
	this->buffer_push(this->buffer_pop(2) + 1);
	return node;
}

std::unique_ptr<RegexAST> RegexParser::parse_literal() {
	std::unique_ptr<UInt> x = this->parse_absolute_literal();
	if (x) {
		std::unique_ptr<RegexAST> r = RegexParser::make_regex_ast_literal(*x);
		return r;
	}
	UInt ch = this->buffer_char();
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

std::unique_ptr<RegexAST> RegexParser::parse_group() {
	if (this->buffer_char() != RegexParser::TOKEN_LBRACKET) {
		return nullptr;
	}
	this->buffer_advance(1);
	std::unique_ptr<RegexAST> contents = this->parse_group_contents();
	if (!contents) {
		this->buffer_pop(1);
		return nullptr;
	}
	if (this->buffer_char() != RegexParser::TOKEN_RBRACKET) {
		this->buffer_pop(2);
		return nullptr;
	}
	this->buffer_advance(1);
	this->buffer_push(this->buffer_pop(3));
	return contents;
}

std::unique_ptr<RegexAST> RegexParser::parse_group_contents() {
	std::unique_ptr<RegexAST> car = this->parse_group_element();
	if (!car) {
		return nullptr;
	}
	std::unique_ptr<RegexAST> cdr = this->parse_group_contents();
	if (!cdr) {
		return car;
	}
	this->buffer_push(this->buffer_pop(2));
	return RegexParser::make_regex_ast_or(&car, &cdr);
}

std::unique_ptr<RegexAST> RegexParser::parse_group_element() {
	if (std::unique_ptr<RegexAST> node = this->parse_group_range()) {
		return node;
	} else if (std::unique_ptr<UInt> x = this->parse_group_literal()) {
		 std::unique_ptr<RegexAST> r = RegexParser::make_regex_ast_literal(*x);
		 return r;
	}
	return nullptr;
}

std::unique_ptr<UInt> RegexParser::parse_group_literal() {
	std::unique_ptr<UInt> l = this->parse_absolute_literal();
	if (l) {
		return l;
	}
	Int ch = this->buffer_char();
	if (ch == RegexParser::TOKEN_ESCAPE) {
		ch = this->buffer_char(1);
		if (RegexParser::is_group_special_char(ch)) {
			this->buffer_advance(2);
			return std::unique_ptr<UInt>(new UInt(ch));
		}
		return nullptr;
	}
	if (!RegexParser::is_group_special_char(ch) && 32 <= ch && ch < 127) {
		this->buffer_advance(1);
		return std::unique_ptr<UInt>(new UInt(ch));
	}
	return nullptr;
}

std::unique_ptr<RegexAST> RegexParser::parse_group_range() {
	std::unique_ptr<UInt> lower = this->parse_group_literal();
	if (!lower) {
		return nullptr;
	}
	if (this->buffer_char() != '-') {
		this->buffer_pop(1);
		return nullptr;
	}
	this->buffer_advance(1);
	std::unique_ptr<UInt> upper = this->parse_group_literal();
	if (!upper) {
		this->buffer_pop(2);
		return nullptr;
	}
	std::unique_ptr<RegexAST> node = RegexParser::make_regex_ast_range(*lower, *upper);
	this->buffer_push(this->buffer_pop(3));
	return node;
}

std::unique_ptr<UInt> RegexParser::parse_absolute_literal() {
	if (this->buffer_char() != RegexParser::TOKEN_ESCAPE) {
		return nullptr;
	}
	this->buffer_advance(1);
	UInt ch = this->buffer_char();
	if (ch == RegexParser::TOKEN_X) {
		this->buffer_advance(1);
		std::unique_ptr<UInt> x = this->parse_hex_byte();
		if (!x) {
			this->buffer_pop(2);
			return nullptr;
		}
		this->buffer_push(this->buffer_pop(2));
		return x;
	} else if (ch == RegexParser::TOKEN_U) {
		this->buffer_advance(1);
		std::unique_ptr<UInt> x = this->parse_hex_int();
		if (!x) {
			this->buffer_pop(2);
			return nullptr;
		}
		this->buffer_push(this->buffer_pop(2));
		return x;
	} else if (ch == RegexParser::TOKEN_T) {
		this->buffer_advance(1);
		this->buffer_push(this->buffer_pop(2));
		return std::unique_ptr<UInt>(new UInt('\t'));
	} else if (ch == RegexParser::TOKEN_N) {
		this->buffer_advance(1);
		this->buffer_push(this->buffer_pop(2));
		return std::unique_ptr<UInt>(new UInt('\n'));
	} else if (ch == RegexParser::TOKEN_R) {
		this->buffer_advance(1);
		this->buffer_push(this->buffer_pop(2));
		return std::unique_ptr<UInt>(new UInt('\r'));
	}
	this->buffer_pop(1);
	return nullptr;
}

std::unique_ptr<UInt> RegexParser::parse_hex_byte() {
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
	return std::unique_ptr<UInt>(new UInt(x));
}

std::unique_ptr<UInt> RegexParser::parse_hex_int() {
	UInt x = 0;
	this->buffer_advance(0);
	for (Int i = 0; i < 4; i++) {
		std::unique_ptr<UInt> b = this->parse_hex_byte();
		if (!b) {
			this->buffer_pop(1);
			return nullptr;
		}
		x = (x << 8) + *b;
		this->buffer_push(this->buffer_pop(2));
	}
	return std::unique_ptr<UInt>(new UInt(x));
}

std::unique_ptr<UInt> RegexParser::parse_dec_int() {
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
	return std::unique_ptr<UInt>(new UInt(x));
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
}

void RegexNFAGenerator::new_rule(std::string tag) {
	this->target_state = this->nfa.new_state();
	this->target_state->terminal = true;
	this->target_state->data = tag;
}

RegexNFAState* RegexNFAGenerator::next_state() {
	return this->target_state == nullptr ? this->nfa.new_state() : this->target_state;
}

void RegexNFAGenerator::visit(RegexASTLiteral* node) {
	this->root->next_states[node->ch].push_back(this->target_state);
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
		for (UInt i = 1; i < node->min; i++) {
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
		for (UInt i = node->min + 1; i < node->max; i++) {
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
	std::unique_ptr<RegexAST> r(new RegexASTLiteral(node->upper));
	for (UInt i = node->upper - 1; i >= node->lower; i--) {
		std::unique_ptr<RegexAST> r2(new RegexASTOr(std::unique_ptr<RegexAST>(new RegexASTLiteral(i)), std::move(r)));
		r = std::move(r2);
	}
	r->accept(this);
}

//} // namespace mami
