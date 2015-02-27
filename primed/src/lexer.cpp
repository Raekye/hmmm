#include "lexer.h"
#include <iostream>

Lexer::Lexer() : buffer_pos(0), eof(false), current_state(0), regenerate(false), generation_child_state(0), generation_regex_chain_end(NULL) {
	return;
}

Lexer::~Lexer() {
	return;
}

void Lexer::generate() {
	this->clean();
	int32_t root = this->generation_new_state();
	this->generation_parent_states_stack.push(root);
	std::cout << "=== Rules" << std::endl;
	for (int32_t i = 0; i < this->rules.size(); i++) {
		RegexAST* regex = this->regex_parser.parse(this->rules[i].pattern);
		this->generation_terminal_tag = this->rules[i].tag;
		RegexASTPrinter a;
		a.indents = 1;
		std::cout << "Rule " << this->rules[i].name << std::endl;
		regex->accept(&a);
		std::cout << "Endrule " << this->rules[i].name << std::endl;
		if (!regex) {
			throw std::runtime_error("Invalid regex " + this->rules[i].pattern + " for rule " + this->rules[i].name);
		}
		regex->accept(this);
		delete regex;
	}
	std::cout << std::endl;
}

void Lexer::clean() {
	for (int32_t i = 0; i < this->states.size(); i++) {
		delete this->states[i];
	}
	this->states.clear();
	this->generation_parent_states_stack = std::stack<int32_t>();
	this->current_state = 0;
}

void Lexer::add_rule(Rule rule) {
	this->rules.push_back(rule);
	this->regenerate = true;
}

Token* Lexer::scan(std::istream* in) {
	if (this->regenerate) {
		this->generate();
		this->regenerate = false;
	}
	int32_t ch = this->read(in);
	State* current = this->states[this->current_state];
	if (ch == 0) {
		// case: at eof
		if (current->is_terminal()) {
			// case: is terminal
			Token* t = new Token(current->tag, this->buffer.substr(0, this->buffer_pos), LocationInfo(0, 0, 0, 0));
			this->buffer = this->buffer.substr(this->buffer_pos);
			this->buffer_pos = 0;
			return t;
		}
		// elsecase: not terminal
		return NULL;
	}
	// elsecase: not eof
	Token* t = NULL;
	if (current->is_terminal()) {
		t = new Token(current->tag, this->buffer.substr(0, this->buffer_pos), LocationInfo(0, 0, 0, 0));
	}
	std::map<int32_t, std::vector<int32_t>>::iterator it = current->next_states.find(ch);
	if (it == current->next_states.end()) {
		// case: no next
		if (t) {
			// case: is terminal
			this->buffer = this->buffer.substr(this->buffer_pos);
			this->buffer_pos = 0;
		}
		// elsecase: not terminal
		return t;
	}
	// elsecase: has next
	for (int32_t i = 0; i < it->second.size(); i++) {
		this->buffer_pos++;
		this->current_state = it->second[i];
		Token* t2 = this->scan(in);
		if (t2) {
			if (!t) {
				t = t2;
			} else if (t->lexeme.length() < t2->lexeme.length()) {
				delete t;
				t = t2;
			} else {
				delete t2;
			}
		}
		this->buffer_pos--;
	}
	return t;
}

int32_t Lexer::read(std::istream* in) {
	if (this->buffer_pos >= this->buffer.length()) {
		if (this->eof) {
			return 0;
		}
		int32_t ch = in->get();
		if (in->good()) {
			this->buffer.push_back(ch);
		} else {
			this->eof = true;
			return 0;
		}
	}
	return this->buffer[this->buffer_pos];
}

int32_t Lexer::generation_parent_state() {
	return this->generation_parent_states_stack.top();
}

int32_t Lexer::generation_new_state() {
	State* s = new State;
	int32_t next_index = this->states.size();
	this->states.push_back(s);
	return next_index;
}

void Lexer::print_states() {
	std::cout << "=== States" << std::endl;
	for (int32_t i = 0; i < this->states.size(); i++) {
		std::cout << "State " << i << (this->states[i]->is_terminal() ? "(end)" : "") << ": " << this->states[i]->tag;
		for (std::map<int32_t, std::vector<int32_t>>::iterator it = this->states[i]->next_states.begin(); it != this->states[i]->next_states.end(); it++) {
			std::cout << ", " << (char) it->first << " ->";
			for (int32_t j = 0; j < it->second.size(); j++) {
				std::cout << " " << it->second[j];
			}
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
}

#pragma mark - IRegexASTVisitor methods
void Lexer::visit(RegexASTChain* x) {
	this->generation_parent_states_stack.push(this->generation_parent_state());
	for (int32_t i = 0; i < x->sequence.size(); i++) {
		int32_t next_state = this->generation_new_state();
		this->generation_child_state = next_state;
		x->sequence.operator[](i)->accept(this);
		this->generation_parent_states_stack.pop();
		this->generation_parent_states_stack.push(this->generation_child_state);
		if (x->sequence.operator[](i) == this->generation_regex_chain_end) {
			this->states[this->generation_child_state]->tag = this->generation_terminal_tag;
		}
	}
	this->generation_parent_states_stack.pop();
}

void Lexer::visit(RegexASTLiteral* x) {
	State* parent = this->states[this->generation_parent_state()];
	std::map<int32_t, std::vector<int32_t>>::iterator it = parent->next_states.find(x->ch);
	if (it == parent->next_states.end()) {
		parent->next_states[x->ch].push_back(this->generation_child_state);
	} else {
		it->second.push_back(this->generation_child_state);
	}
}

void Lexer::visit(RegexASTOr* x) {
	x->left->accept(this);
	x->right->accept(this);
}

void Lexer::visit(RegexASTMultiplication* x) {
	this->generation_child_state = this->generation_parent_state();
	x->node->accept(this);
}

void Lexer::visit(RegexASTRange* x) {
	State* parent = this->states[this->generation_parent_state()];
	for (int32_t ch = x->lower; ch <= x->upper; ch++) {
		std::map<int32_t, std::vector<int32_t>>::iterator it = parent->next_states.find(ch);
		if (it == parent->next_states.end()) {
			parent->next_states[ch].push_back(this->generation_child_state);
		} else {
			it->second.push_back(this->generation_child_state);
		}
	}
}
