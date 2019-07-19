#ifndef MIDORI_GENERATOR_H_INCLUDED
#define MIDORI_GENERATOR_H_INCLUDED

#include "regex.h"
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

class RegexParserGenerator {
public:
	static std::unique_ptr<Parser> make();

private:
	static void add_literal(Parser*, std::string, std::string, UInt);
};

#endif /* MIDORI_GENERATOR_H_INCLUDED */