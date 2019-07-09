#include "regex.h"
#include <cctype>
#include <iterator>

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

RegexASTWildcard::RegexASTWildcard() {
	return;
}

RegexASTNot::RegexASTNot(std::unique_ptr<RegexAST> node) : node(std::move(node)) {
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

void RegexASTWildcard::accept(IRegexASTVisitor* visitor) {
	visitor->visit(this);
}

void RegexASTNot::accept(IRegexASTVisitor* visitor) {
	visitor->visit(this);
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
	this->target_state = nullptr;
	this->inversion = false;
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
	this->transitions()->operator[](node->ch).push_back(this->target_state);
}

void RegexNFAGenerator::visit(RegexASTWildcard* node) {
	(void) node;
	this->transitions()->operator[](-1).push_back(this->target_state);
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

void RegexNFAGenerator::visit(RegexASTNot* node) {
	(void) node;
	// TODO: the following is incorrect
	/*
	this->inversion = !this->inversion;
	node->node->accept(this);
	this->inversion = !this->inversion;
	*/
}
