#ifndef MIDORI_REGEX_ENGINE_H_INCLUDED
#define MIDORI_REGEX_ENGINE_H_INCLUDED

#include "regex_ast.h"
#include "parser.h"

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
