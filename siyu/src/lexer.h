#ifndef SIYU_LEXER_H_INCLUDED
#define SIYU_LEXER_H_INCLUDED

#include <cstdint>
#include <string>
#include <map>
#include <vector>
#include <istream>
#include <functional>
#include <stack>
#include "regex.h"

struct LocationInfo {
	int32_t start_line;
	int32_t start_column;
	int32_t end_line;
	int32_t end_column;
	LocationInfo(int32_t start_line, int32_t start_column, int32_t end_line, int32_t end_column) : start_line(start_line), start_column(start_column), end_line(end_line), end_column(end_column) {
		return;
	}
};

struct Token {
	std::string tag;
	std::string lexeme;
	LocationInfo loc;
	Token(std::string tag, std::string lexeme, LocationInfo loc) : tag(tag), lexeme(lexeme), loc(loc) {
		return;
	}
};

struct Rule {
	std::string tag;
	std::string pattern;
	Rule(std::string tag, std::string pattern) : tag(tag), pattern(pattern) {
		return;
	}
};

class Lexer {
private:
	std::vector<Rule> rules;
	bool regenerate;
	RegexParser regex_parser;

	RegexNFAGenerator regex_nfa_generator;
	std::unique_ptr<RegexDFA> dfa;
	RegexDFAState* current_state;

	std::string buffer;
	int32_t buffer_pos;
	bool eof;

	UInt read(std::istream*);
	void clean();
	void generate();
public:
	Lexer();

	void add_rule(Rule);
	Token* scan(std::istream*);
};

#endif /* SIYU_LEXER_H_INCLUDED */
