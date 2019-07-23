#include "gtest/gtest.h"
#include "midori/parser.h"
#include <sstream>

class ParserTest : public ::testing::Test {
};

TEST_F(ParserTest, Basic) {
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
}

TEST_F(ParserTest, Recursion) {
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
}

TEST_F(ParserTest, Epsilon) {
	ProductionHandler fn = [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		(void) m;
		return nullptr;
	};
	Parser p;
	p.add_token("A", std::unique_ptr<RegexAST>(new RegexASTLiteral('a')));
	p.add_production("n", { "m", "n" }, fn);
	p.add_production("n", {}, fn);
	p.add_production("m", { "A" }, fn);
	p.generate("n");
	std::stringstream ss;
	ss << "aaa";
	FileInputStream fis(&ss);
	p.parse(&fis);
	p.reset();
	ss << "";
	p.parse(&fis);
}

TEST_F(ParserTest, RegexGroup) {
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
}
