#include "lexer.h"

Lexer::Lexer() : buffer_pos(0), eof(false), current_state(0) {
	return;
}

Lexer::~Lexer() {
	return;
}

void Lexer::generate() {
	State root;
	this->states.push_back(root);
	for (int32_t i = 0; i < this->rules.size(); i++) {
		this->generate_rule(root, this->rules[i], 0);
	}
}

void Lexer::clean() {
	this->rules.clear();
	this->states.clear();
}

void Lexer::add_rule(int32_t index, Rule rule) {
	this->rules.insert(this->rules.begin() + index, rule);
}

Token* Lexer::scan(std::istream in) {
	if (this->buffer_pos >= this->buffer.length()) {
		int32_t ch = in.get();
		if (in.good()) {
			this->buffer.push_back(ch);
		} else {
			this->eof = true;
		}
	}
	if (this->eof) {
	} else {
		State current = this->states[this->current_state];
		if (current.is_terminal()) {
			// TODO: found match
		}
		int32_t ch = this->buffer[buffer_pos];
		std::map<int32_t, std::vector<int32_t>>::iterator it = current.next_states.find(ch);
		if (it == current.next_states.end()) {
			// TODO: not match
		} else {
			// TODO: loop through next_states, try
		}
	}
	return NULL;
}

void Lexer::generate_rule(State parent, Rule rule, int32_t pattern_pos)  {
	if (pattern_pos >= rule.pattern.length()) {
		parent.tag = rule.tag;
		return;
	}
	std::string car = rule.pattern.substr(pattern_pos, 1);
	int32_t ch = car[0];
	int32_t next_pattern_pos = pattern_pos + 1;
	std::map<int32_t, std::vector<int32_t>>::iterator it = parent.next_states.find(ch);
	if (it == parent.next_states.end()) {
		std::vector<int32_t> next_states_for_char;
		next_states_for_char.push_back(this->states.size());
		State current;
		this->states.push_back(current);
		it->second = next_states_for_char; // TODO: this ok?
		this->generate_rule(current, rule, next_pattern_pos);
	} else {
		// TODO: follow chain
	}
}
