#include <sstream>
#include <fstream>
#include "midori/helper.h"
#include "midori/regex.h"
//#include "midori/lexer.h"
//#include "midori/parser.h"
//#include "midori/generator.h"
#include "midori/interval_tree.h"

int test_parser() {
	/*
	std::unique_ptr<Parser> p = RegexParserGenerator::make();

	for (int i = 0; i < 2; i++) {
		std::stringstream ss;
		ss << "a(bc){3,4}\\[|def[ghi\\t0-9\\-]+\\000000x2dz(\\x2d)d";
		//ss << "[x-zabc-f]";
		std::unique_ptr<Match> m = p->parse(&ss);
		MatchedNonterminal* n = dynamic_cast<MatchedNonterminal*>(m.get());
		ParserRegexAST* r = dynamic_cast<ParserRegexAST*>(n->value.get());
		RegexASTPrinter printer;
		r->regex->accept(&printer);
	}
	*/
	return 0;
}

/*
int test_parser() {
	std::string indents = "";
	ProductionHandler fn = [&indents, &fn](Match* m) -> void {
		std::stack<Match*> s;
		s.push(m);
		while (s.size() > 0) {
			m = s.top();
			s.pop();
			if (MatchedTerminal* mt = dynamic_cast<MatchedTerminal*>(m)) {
				mdk::logf("%s- terminal %s, %s\n", indents.c_str(), mt->token->tag.c_str(), mt->token->lexeme.c_str());
			} else if (MatchedNonterminal* mnt = dynamic_cast<MatchedNonterminal*>(m)) {
				mdk::logf("%s- nonterminal ", indents.c_str());
				Parser::debug_production(mnt->production);
				indents += "  ";
				for (std::unique_ptr<Match>& x : mnt->terms) {
					fn(x.get());
				}
				indents = indents.substr(0, indents.length() - 2);
			}
		}
	};
	Parser parser;
	parser.set_start("s");
	parser.add_token("STAR", "\\*");
	parser.add_token("X", "x");
	parser.add_token("EQUALS", "=");
	parser.add_production("s", { "n" }, fn);
	parser.add_production("n", { "v", "EQUALS", "e" }, fn);
	parser.add_production("n", { "e" }, fn);
	parser.add_production("e", { "v" }, fn);
	parser.add_production("v", { "X" }, fn);
	parser.add_production("v", { "STAR", "e" }, fn);
	std::stringstream ss;
	ss << "x=*x";
	parser.parse(&ss);
	return 0;
}

int test_generator() {
	std::fstream f("src/parser.txt", std::fstream::in);
	std::unique_ptr<Parser> p = Parser::from_file(&f);
	(void) p;
	return 0;
}
*/

int test_interval_tree() {
	typedef IntervalTree<UInt, Int> Foo;
	Foo a;
	for (UInt i = 0; i < 100; i++) {
		a.insert(Foo::Interval(i, i + 10), i);
	}
	a.invariants();
	std::unique_ptr<Foo::SearchList> all = a.all();
	std::cout << all->size() << std::endl;

	std::unique_ptr<Foo::SearchList> results = a.pop(Foo::Interval(3, 3));
	a.invariants();
	std::cout << results->size() << std::endl;

	results = a.pop(Foo::Interval(50, 51));
	a.invariants();
	std::cout << results->size() << std::endl;

	Foo b;
	for (UInt i = 0; i < 1000; i++) {
		b.insert(Foo::Interval(i, i), i);
	}
	b.invariants();
	results = b.pop(Foo::Interval(800, 899));
	b.invariants();
	std::cout << results->size() << std::endl;
	return 0;
}

int main() {
	ULong x = ~0;
	std::cout << "-1 is " << x << std::endl;
	test_interval_tree();
	//test_parser();
	//test_generator();
	return 0;
}
