#include <sstream>
#include <fstream>
#include "midori/helper.h"
#include "midori/regex_ast.h"
//#include "midori/lexer.h"
#include "midori/parser.h"
#include "midori/regex_engine.h"
#include "midori/interval_tree.h"
#include "midori/generator.h"

int test_regex_engine() {
	RegexEngine re;
	//std::string pattern = "(abc){0,3}[a-zA-Z]|def.\\.[^a-zA-Z]?+-^\\n+[^\\t\\xff-\\u12345678]";
	std::string pattern = "[-a-b-cd---]{3}abc";
	std::unique_ptr<RegexAST> r = re.parse(pattern);
	RegexASTPrinter printer;
	r->accept(&printer);
	return 0;
}

int test_generator() {
	std::fstream f("../src/parser.txt", std::fstream::in);
	std::unique_ptr<Parser> p = ParserGenerator::from_file(&f);
	std::stringstream ss;
	ss << "a1ab2bc3cd4d";
	FileInputStream fis(&ss);
	std::unique_ptr<MatchedNonterminal> m = p->parse(&fis);
	assert(m != nullptr);
	return 0;
}

int main() {
	ULong x = ~0;
	std::cout << "-1 is " << x << std::endl;
	test_generator();
	test_regex_engine();
	return 0;
}
