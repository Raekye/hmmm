#ifndef MIDORI_REGEX_ENGINE_H_INCLUDED
#define MIDORI_REGEX_ENGINE_H_INCLUDED

#include "regex_ast.h"
#include "parser.h"

class ParserRegexAST : public ParserAST {
public:
	std::unique_ptr<RegexAST> regex;
	ParserRegexAST(std::unique_ptr<RegexAST>);
};

class ParserStringAST : public ParserAST {
public:
	std::string str;
	ParserStringAST(std::string);
};

class ParserRangeAST : public ParserAST {
public:
	UInt min;
	UInt max;
	ParserRangeAST(Long, Long);
};

class RegexEngine {
public:
	RegexEngine();
	std::unique_ptr<RegexAST> parse(std::string);

private:
	std::unique_ptr<Parser> parser;

	static std::unique_ptr<Parser> make();
	static void add_literal(Parser*, std::string, std::string, UInt);
};

#endif /* MIDORI_REGEX_ENGINE_H_INCLUDED */
