#ifndef TK_LEXER_H_INCLUDED
#define TK_LEXER_H_INCLUDED

#include <cstdint>
#include <string>
#include <map>
#include <vector>
#include <istream>
#include <functional>
#include <stack>
#include "global.h"
#include "regex.h"

struct LocationInfo {
	UInt line;
	UInt column;
	LocationInfo(UInt line, UInt column) : line(line), column(column) {
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
	UInt buffer_pos;
	bool eof;

	UInt read(std::istream*);
	void clean();
	void generate();
	void prepare();
public:
	Lexer();

	void add_rule(Rule);
	std::unique_ptr<Token> scan(std::istream*);
};

#endif /* TK_LEXER_H_INCLUDED */
