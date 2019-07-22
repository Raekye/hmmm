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
#include "regex_ast.h"

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

class VectorInputStream : public IInputStream {
public:
	VectorInputStream(std::vector<UInt>);
	Long get() override;

private:
	std::vector<UInt> v;
	size_t pos;
};

class Lexer {
public:
	static std::string const TOKEN_END;
	static std::string const TOKEN_BAD;
	static const Long CHAR_EOF = -1;
	static const Long CHAR_BAD = -2;

	Lexer();

	void add_rule(std::string, std::unique_ptr<RegexAST>);
	std::unique_ptr<Token> scan(IInputStream*);
	void generate();
	void reset();

private:
	std::vector<std::string> rules;
	std::vector<std::unique_ptr<RegexAST>> rules_regex;

	std::unique_ptr<RegexDFA> dfa;
	RegexDFAState* current_state;

	std::vector<UInt> buffer;
	UInt buffer_pos;

	LocationInfo location;

	Long read(IInputStream*);
};

#endif /* MIDORI_LEXER_H_INCLUDED */
