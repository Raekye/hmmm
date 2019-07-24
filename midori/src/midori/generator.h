#ifndef MIDORI_GENERATOR_H_INCLUDED
#define MIDORI_GENERATOR_H_INCLUDED

#include "parser.h"
#include "regex_engine.h"
#include <istream>

class ParserGenerator {
public:
	static std::unique_ptr<Parser> from_file(std::istream*);
};

#endif /* MIDORI_GENERATOR_H_INCLUDED */
