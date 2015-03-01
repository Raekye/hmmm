#include "lexer.h"
#include <iostream>

Lexer::Lexer() : regenerate(false), buffer_pos(0), eof(false) {
	this->current_state = 0;
}

void Lexer::generate() {
	if (!this->regenerate) {
		return;
	}
	this->regenerate = false;
	std::cout << "=== Rules" << std::endl;
	for (int32_t i = 0; i < this->rules.size(); i++) {
		std::cout << "Rule " << i << " - " << this->rules[i].tag << " - " << this->rules[i].pattern << std::endl;
		std::unique_ptr<RegexAST> regex = this->regex_parser.parse(this->rules[i].pattern);
		if (!regex) {
			throw std::runtime_error("Invalid regex " + this->rules[i].pattern + " for rule " + this->rules[i].tag);
		}

		RegexASTPrinter rp;
		rp.indents = 2;
		std::cout << "\tRegex AST" << std::endl;
		regex->accept(&rp);
		std::cout << "\tEnd regex ast" << std::endl;
		std::cout << std::endl;

		this->regex_nfa_generator.new_rule(this->rules[i].tag);
		regex->accept(&(this->regex_nfa_generator));

		std::cout << "End rule " << this->rules[i].tag << std::endl;
		std::cout << std::endl;
	}
	std::cout << "=== End rules" << std::endl;
	std::cout << std::endl;

	std::cout << "=== NFA states" << std::endl;
	for (auto& state : this->regex_nfa_generator.nfa.states) {
		std::cout << "\tNFA state " << state->id;
		if (state->terminal) {
			std::cout << " terminal '" << state->data << "'";
		}
		std::cout << std::endl;
		for (auto& kv : state->next_states) {
			std::cout << "\t\tChar " << (char) kv.first << ": ";
			for (auto& next_state : kv.second) {
				std::cout << " " << next_state->id << ",";
			}
			std::cout << std::endl;
		}
		if (state->epsilon != nullptr) {
			std::cout << "\t\tEpsilon to " << state->epsilon->id << std::endl;
		}
	}
	std::cout << "=== End nfa states" << std::endl;
	std::cout << std::endl;

	std::cout << "=== DFA states" << std::endl;
	this->dfa = this->regex_nfa_generator.nfa.to_dfa();
	for (auto& state : this->dfa->states) {
		std::cout << "\tDFA state " << state->id;
		if (state->terminal) {
			std::cout << " terminal '" << state->data << "'";
		}
		std::cout << std::endl;
		for (auto& kv : state->next_states) {
			std::cout << "\t\tChar " << (char) kv.first << " to " << kv.second->id << std::endl;
		}
	}
	std::cout << "=== End dfa states" << std::endl;
	std::cout << std::endl;
}

void Lexer::clean() {
	//this->regex_nfa_generator.reset();
}

void Lexer::add_rule(Rule rule) {
	this->rules.push_back(rule);
	this->regenerate = true;
}

Token* Lexer::scan(std::istream* in) {
	this->generate();
	/*
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
		return nullptr;
	}
	// elsecase: not eof
	Token* t = nullptr;
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
	*/
	return nullptr;
}

UInt Lexer::read(std::istream* in) {
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
