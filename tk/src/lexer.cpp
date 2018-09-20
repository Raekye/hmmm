#include "lexer.h"
#include <iostream>

Lexer::Lexer() : regenerate(false), current_state(nullptr), buffer_pos(0), eof(false) {
	return;
}

void Lexer::generate() {
	std::cout << "=== Rules" << std::endl;
	for (size_t i = 0; i < this->rules.size(); i++) {
		std::cout << "Rule " << i << " - " << this->rules[i].tag << " - " << this->rules[i].pattern << std::endl;

		RegexASTPrinter rp;
		rp.indents = 2;
		std::cout << "\tRegex AST" << std::endl;
		this->rules_regex[i]->accept(&rp);
		std::cout << "\tEnd regex ast" << std::endl;
		std::cout << std::endl;

		this->regex_nfa_generator.new_rule(this->rules[i].tag);
		this->rules_regex[i]->accept(&(this->regex_nfa_generator));

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
	this->regex_nfa_generator.reset();
}

void Lexer::prepare() {
	if (this->regenerate) {
		this->clean();
		this->generate();
		this->regenerate = false;
	}
	this->current_state = this->dfa == nullptr ? nullptr : this->dfa->root;
}

bool Lexer::add_rule(Rule rule) {
	std::unique_ptr<RegexAST> regex = this->regex_parser.parse(rule.pattern);
	if (!regex) {
		return false;
	}
	this->rules_regex.push_back(std::move(regex));
	this->rules.push_back(rule);
	this->regenerate = true;
	return true;
}

std::unique_ptr<Token> Lexer::scan(std::istream* in) {
	this->prepare();
	if (this->current_state == nullptr) {
		return nullptr;
	}
	bool matched = false;
	std::string matched_tag = "-";
	std::string matched_str = "";
	UInt matched_buffer_pos = this->buffer_pos;
	std::string found_buffer = "";
	std::unique_ptr<Token> t;
	while (true) {
		if (this->current_state->terminal) {
			matched = true;
			matched_tag = this->current_state->data;
			matched_str.append(found_buffer);
			matched_buffer_pos = this->buffer_pos;
			found_buffer = "";
		}
		UInt ch = this->read(in);
		std::map<UInt, RegexDFAState*>::iterator it = this->current_state->next_states.find(ch);
		if (ch == 0 || it == this->current_state->next_states.end()) {
			if (matched) {
				t.reset(new Token(matched_tag, matched_str, LocationInfo(0, 0)));
				break;
			}
			break;
		}
		found_buffer.append(1, (char) ch);
		this->buffer_pos++;
		this->current_state = it->second;
	}
	this->buffer_pos = matched_buffer_pos;
	return t;
}

UInt Lexer::read(std::istream* in) {
	if (this->buffer_pos >= this->buffer.length()) {
		if (this->eof) {
			return 0;
		}
		UInt ch = in->get();
		if (in->good()) {
			this->buffer.push_back(ch);
		} else {
			this->eof = true;
			return 0;
		}
	}
	return this->buffer[this->buffer_pos];
}
