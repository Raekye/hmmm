#include "lexer.h"
#include <iostream>
#include "utf8.h"

std::string const Lexer::TOKEN_END = "$END";
std::string const Lexer::TOKEN_BAD = "$BAD";

IInputStream::~IInputStream() {
	return;
}

FileInputStream::FileInputStream(std::istream* file) : file(file) {
	return;
}

Long FileInputStream::get() {
	Long ch = utf8::codepoint_from_istream(this->file);
	if (ch < 0) {
		if (this->file->eof()) {
			return Lexer::CHAR_EOF;
		}
		return Lexer::CHAR_BAD;
	}
	return ch;
}

VectorInputStream::VectorInputStream(std::vector<UInt> v) : v(v), pos(0) {
	return;
}

Long VectorInputStream::get() {
	if (this->pos >= this->v.size()) {
		return Lexer::CHAR_EOF;
	}
	return this->v.at(this->pos++);
}

Lexer::Lexer() {
	this->reset();
}

void Lexer::generate() {
	RegexNFAGenerator regex_nfa_generator;
	std::cout << "=== Rules" << std::endl;
	for (size_t i = 0; i < this->rules.size(); i++) {
		std::cout << "Rule " << i << " - " << this->rules[i] << std::endl;

		RegexASTPrinter rp;
		rp.indents = 2;
		std::cout << "\tRegex AST" << std::endl;
		this->rules_regex[i]->accept(&rp);
		std::cout << "\tEnd regex ast" << std::endl;

		regex_nfa_generator.new_rule(this->rules[i]);
		this->rules_regex[i]->accept(&regex_nfa_generator);

		std::cout << "End rule " << this->rules[i] << std::endl;
	}
	std::cout << "=== End rules" << std::endl;
	std::cout << std::endl;

	std::cout << "=== NFA states" << std::endl;
	for (auto& state : regex_nfa_generator.nfa.states) {
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
	this->dfa = regex_nfa_generator.nfa.to_dfa();
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

void Lexer::reset() {
	this->buffer.clear();
	this->buffer_pos = 0;
	this->location.line = 1;
	this->location.column = 1;
}

void Lexer::add_rule(std::string rule, std::unique_ptr<RegexAST> regex) {
	this->rules.push_back(rule);
	this->rules_regex.push_back(std::move(regex));
}

void Lexer::add_skip(std::string symbol) {
	this->skip.insert(symbol);
}

std::unique_ptr<Token> Lexer::scan(IInputStream* in) {
	// TODO: trailing newline in files?
	while (true) {
		std::unique_ptr<Token> t = this->_scan(in);
		Int i = 0;
		for (std::string const& s : t->tags) {
			if (this->skip.find(s) != this->skip.end()) {
				break;
			}
			i++;
		}
		if (i == t->tags.size()) {
			return t;
		}
	}
	return nullptr;
}

std::unique_ptr<Token> Lexer::_scan(IInputStream* in) {
	RegexDFAState* current_state = this->dfa->root();
	bool matched = false;
	std::vector<std::string> matched_tags;
	std::string matched_str = "";
	UInt matched_buffer_pos = this->buffer_pos;
	LocationInfo saved_location = this->location;
	LocationInfo matched_location = this->location;
	std::string found_buffer = "";
	std::unique_ptr<Token> t;
	while (true) {
		if (!current_state->terminals.empty()) {
			matched = true;
			matched_tags = current_state->terminals;
			matched_str.append(found_buffer);
			matched_buffer_pos = this->buffer_pos;
			matched_location = this->location;
			found_buffer = "";
		}
		Long ch = this->read(in);
		if (ch < 0) {
			if (matched) {
				t.reset(new Token(matched_tags, matched_str, saved_location));
				break;
			}
			if (ch == Lexer::CHAR_EOF) {
				if (this->buffer_pos == matched_buffer_pos) {
					t.reset(new Token({ Lexer::TOKEN_END }, "", this->location));
					break;
				}
			}
			t.reset(new Token({ Lexer::TOKEN_BAD }, found_buffer, saved_location));
			break;
		}
		found_buffer.append(utf8::string_from_codepoint(ch));
		this->buffer_pos++;
		if (ch == '\n') {
			this->location.line++;
			this->location.column = 1;
		} else {
			this->location.column++;
		}
		RegexDFAState* next = current_state->next(ch);
		if (next == nullptr) {
			if (matched) {
				t.reset(new Token(matched_tags, matched_str, saved_location));
			} else {
				t.reset(new Token({ Lexer::TOKEN_BAD }, found_buffer, saved_location));
			}
			break;
		}
		current_state = next;
	}
	this->buffer_pos = matched_buffer_pos;
	this->location = matched_location;
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
