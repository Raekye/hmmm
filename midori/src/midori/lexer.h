#ifndef MIDORI_LEXER_H_INCLUDED
#define MIDORI_LEXER_H_INCLUDED

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
	std::vector<std::string> tags;
	std::string lexeme;
	LocationInfo loc;
	Token(std::vector<std::string> tags, std::string lexeme, LocationInfo loc) : tags(tags), lexeme(lexeme), loc(loc) {
		return;
	}
};

class IInputStream {
public:
	virtual ~IInputStream();
	virtual Long get() = 0;
};

class FileInputStream : public IInputStream {
public:
	FileInputStream(std::istream*);
	virtual Long get() override;

private:
	std::istream* file;
};

class Lexer {
private:
	std::vector<std::string> rules;
	std::vector<std::unique_ptr<RegexAST>> rules_regex;
	bool regenerate;

	RegexNFAGenerator regex_nfa_generator;
	std::unique_ptr<RegexDFA> dfa;
	RegexDFAState* current_state;

	std::vector<UInt> buffer;
	UInt buffer_pos;

	Long read(IInputStream*);
	void clean();
	void prepare();
public:
	Lexer();

	void add_rule(std::string, std::unique_ptr<RegexAST>);
	std::unique_ptr<Token> scan(IInputStream*);
	void generate();
	void reset();
};

#endif /* MIDORI_LEXER_H_INCLUDED */
