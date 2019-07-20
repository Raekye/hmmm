#include "lexer.h"
#include <iostream>
#include "utf8.h"

IInputStream::~IInputStream() {
	return;
}

FileInputStream::FileInputStream(std::istream* file) : file(file) {
	return;
}

Long FileInputStream::get() {
	UInt ch = this->file->get();
	if (!this->file->good()) {
		return -1;
	}
	return ch;
}

Lexer::Lexer() : regenerate(false), current_state(nullptr), buffer_pos(0) {
	return;
}

void Lexer::generate() {
	std::cout << "=== Rules" << std::endl;
	for (size_t i = 0; i < this->rules.size(); i++) {
		std::cout << "Rule " << i << " - " << this->rules[i] << std::endl;

		RegexASTPrinter rp;
		rp.indents = 2;
		std::cout << "\tRegex AST" << std::endl;
		this->rules_regex[i]->accept(&rp);
		std::cout << "\tEnd regex ast" << std::endl;

		this->regex_nfa_generator.new_rule(this->rules[i]);
		this->rules_regex[i]->accept(&(this->regex_nfa_generator));

		std::cout << "End rule " << this->rules[i] << std::endl;
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
		for (UInt i = 0; i < RegexDFAState::OPTIMIZED_CHARS; i++) {
			if (!state->_transitions[i].empty()) {
				std::cout << "\t\tChar ";
				if ((32 <= i) && (i < 127)) {
					std::cout << (char) i;
				} else {
					std::cout << "\\" << i;
				}
				std::cout << " to";
				for (RegexNFAState* const s : state->_transitions[i]) {
					std::cout << " " << s->id;
				}
				std::cout << std::endl;
			}
		}
		std::unique_ptr<RegexNFAState::UnicodeIntervalTree::SearchList> l = state->transitions.all();
		if (!l->empty()) {
			for (RegexNFAState::UnicodeIntervalTree::SearchList::iterator it = l->begin(); it != l->end(); it++) {
				std::cout << "\t\tRange " << (*it).first.first << " - " << (*it).first.second << " to";
				for (RegexNFAState* const s : (*it).second) {
					std::cout << " " << s->id;
				}
			}
			std::cout << std::endl;
		}
		if (!state->epsilon.empty()) {
			std::cout << "\t\tEpsilon to";
			for (RegexNFAState* const s : state->epsilon) {
				std::cout << " " << s->id;
			}
			std::cout << std::endl;
		}
	}
	std::cout << "=== End nfa states" << std::endl;
	std::cout << std::endl;

	std::cout << "=== DFA states" << std::endl;
	this->dfa = this->regex_nfa_generator.nfa.to_dfa();
	for (auto& state : this->dfa->states) {
		std::cout << "\tDFA state " << state->id;
		if (!state->terminals.empty()) {
			std::cout << " terminal";
			for (std::string const& s : state->terminals) {
				std::cout << " '" << s << "'";
			}
		}
		std::cout << std::endl;
		for (UInt i = 0; i < RegexDFAState::OPTIMIZED_CHARS; i++) {
			if (state->_transitions[i] != nullptr) {
				std::cout << "\t\tChar ";
				if ((32 <= i) && (i < 127)) {
					std::cout << (char) i;
				} else {
					std::cout << "\\" << i;
				}
				std::cout << " to " << state->_transitions[i]->id << std::endl;
			}
		}
		std::unique_ptr<RegexDFAState::Tree::SearchList> l = state->transitions.all();
		for (RegexDFAState::Tree::SearchList::iterator it = l->begin(); it != l->end(); it++) {
			std::cout << "\t\tRange " << (*it).first.first << " - " << (*it).first.second << " to " << (*it).second->id << std::endl;
		}
		std::cout << std::endl;
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
	this->current_state = this->dfa == nullptr ? nullptr : this->dfa->root();
}

void Lexer::reset() {
	this->buffer.clear();
	this->buffer_pos = 0;
	this->current_state = this->dfa == nullptr ? nullptr : this->dfa->root();
}

void Lexer::add_rule(std::string rule, std::unique_ptr<RegexAST> regex) {
	this->rules.push_back(rule);
	this->rules_regex.push_back(std::move(regex));
	this->regenerate = true;
}

std::unique_ptr<Token> Lexer::scan(IInputStream* in) {
	this->prepare();
	if (this->current_state == nullptr) {
		return nullptr;
	}
	bool matched = false;
	std::vector<std::string> matched_tags;
	std::string matched_str = "";
	UInt matched_buffer_pos = this->buffer_pos;
	std::string found_buffer = "";
	std::unique_ptr<Token> t;
	while (true) {
		if (!this->current_state->terminals.empty()) {
			matched = true;
			matched_tags = this->current_state->terminals;
			matched_str.append(found_buffer);
			matched_buffer_pos = this->buffer_pos;
			found_buffer = "";
		}
		Long ch = this->read(in);
		RegexDFAState* next = nullptr;
		if (ch >= 0) {
			if (ch < RegexDFAState::OPTIMIZED_CHARS) {
				next = this->current_state->_transitions[ch];
			} else {
				std::unique_ptr<RegexDFAState::Tree::SearchList> l = this->current_state->transitions.find(RegexDFAState::Tree::Interval(ch, ch));
				assert(l->size() <= 1);
				if (l->size() > 0) {
					next = l->front().second;
				}
			}
		}
		if (next == nullptr) {
			if (matched) {
				t.reset(new Token(matched_tags, matched_str, LocationInfo(0, 0)));
				break;
			}
			break;
		}
		found_buffer.append(utf8::from_codepoint(ch));
		this->buffer_pos++;
		this->current_state = next;
	}
	this->buffer_pos = matched_buffer_pos;
	return t;
}

Long Lexer::read(IInputStream* in) {
	if (this->buffer_pos >= this->buffer.size()) {
		Long ch = in->get();
		if (ch < 0) {
			return ch;
		} else {
			this->buffer.push_back(ch);
		}
	}
	return this->buffer.at(this->buffer_pos);
}
