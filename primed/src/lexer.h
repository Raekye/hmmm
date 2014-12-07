#ifndef PRIMED_LEXER_H_INCLUDED
#define PRIMED_LEXER_H_INCLUDED

#include <cstdint>
#include <string>
#include <map>
#include <vector>
#include <istream>
#include <functional>

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
	std::string name;
	std::string pattern;
	std::string tag;
	Rule(std::string name, std::string pattern, std::string tag) : name(name), pattern(pattern), tag(tag) {
		return;
	}
};

class State {
public:
	std::map<int32_t, std::vector<int32_t>> next_states;
	std::string tag;

	bool is_terminal() {
		return this->tag.length() > 0;
	}
};

class Lexer {
private:
	std::vector<Rule> rules;
	std::vector<State> states;
	std::string buffer;
	int32_t buffer_pos;
	bool eof;
	int32_t current_state;

	void generate_rule(State, Rule, int32_t);

public:
	Lexer();
	virtual ~Lexer();

	void generate();
	void clean();
	void add_rule(int32_t, Rule);
	Token* scan(std::istream);
};

#endif /* PRIMED_LEXER_H_INCLUDED */
