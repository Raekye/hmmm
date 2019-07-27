#include <sstream>
#include <fstream>
#include "midori/helper.h"
#include "midori/regex_ast.h"
//#include "midori/lexer.h"
#include "midori/parser.h"
#include "midori/regex_engine.h"
#include "midori/interval_tree.h"
#include "midori/generator.h"

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

int test_parser0() {
	ProductionHandler fn = [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		(void) m;
		return nullptr;
	};
	Parser p;
	p.add_token("EQUALS", std::unique_ptr<RegexAST>(new RegexASTLiteral('=')));
	p.add_token("X", std::unique_ptr<RegexAST>(new RegexASTLiteral('x')));
	p.add_token("STAR", std::unique_ptr<RegexAST>(new RegexASTLiteral('*')));
	p.add_production("s", { "n" }, fn);
	p.add_production("n", { "v", "EQUALS", "e" }, fn);
	p.add_production("n", { "e" }, fn);
	p.add_production("e", { "v" }, fn);
	p.add_production("v", { "X" }, fn);
	p.add_production("v", { "STAR", "e" }, fn);
	p.generate("s");
	std::stringstream ss;
	ss << "*x=x";
	FileInputStream fis(&ss);
	p.parse(&fis);
	return 0;
}

int test_parser1() {
	ProductionHandler fn = [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		(void) m;
		return nullptr;
	};
	Parser p;
	p.add_token("A", std::unique_ptr<RegexAST>(new RegexASTLiteral('a')));
	p.add_production("n", { "n", "n" }, fn);
	p.add_production("n", { "A" }, fn);
	p.generate("n");
	std::stringstream ss;
	ss << "aaa";
	FileInputStream fis(&ss);
	p.parse(&fis);
	return 0;
}

int test_parser2() {
	ProductionHandler fn = [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		(void) m;
		return nullptr;
	};
	Parser p;
	p.add_token("LB", std::unique_ptr<RegexAST>(new RegexASTLiteral('[')));
	p.add_token("RB", std::unique_ptr<RegexAST>(new RegexASTLiteral(']')));
	p.add_token("DASH", std::unique_ptr<RegexAST>(new RegexASTLiteral('-')));
	p.add_token("ANY", std::unique_ptr<RegexAST>(RegexASTGroup::make(true, { '[', '[', ']', ']' })));
	p.add_production("class", { "LB", "class_contents", "RB" }, fn);
	p.add_production("class_contents", { "class_element" }, fn);
	p.add_production("class_contents", { "class_element", "class_contents" }, fn);
	p.add_production("class_element", { "literal" }, fn);
	p.add_production("class_element", { "class_element", "DASH", "literal" }, fn);
	p.add_production("literal", { "DASH" }, fn);
	p.add_production("literal", { "ANY" }, fn);
	p.generate("class");
	std::stringstream ss;
	ss << "[-a-c-d-]";
	FileInputStream fis(&ss);
	p.parse(&fis);
	return 0;
}

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

int test_lalr() {
	ProductionHandler fn = [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		(void) m;
		return nullptr;
	};
	Parser p;
	p.add_token("EQUALS", std::unique_ptr<RegexAST>(new RegexASTLiteral('=')));
	p.add_token("STAR", std::unique_ptr<RegexAST>(new RegexASTLiteral('*')));
	p.add_token("ID", std::unique_ptr<RegexAST>(new RegexASTLiteral('i')));
	p.add_production("s", { "l", "EQUALS", "r" }, fn);
	p.add_production("s", { "r" }, fn);
	p.add_production("l", { "STAR", "r" }, fn);
	p.add_production("l", { "ID" }, fn);
	p.add_production("r", { "l" }, fn);
	p.generate("s");
	std::stringstream ss;
	ss << "*id=id";
	FileInputStream fis(&ss);
	p.parse(&fis);
	return 0;
}

int main() {
	ULong x = ~0;
	std::cout << "-1 is " << x << std::endl;
	/*
	test_interval_tree();
	test_parser0();
	test_parser2();
	test_parser1();
	test_generator();
	test_regex_engine();
	*/
	test_lalr();
	return 0;
}
