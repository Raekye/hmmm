#include "gtest/gtest.h"
#include "midori/parser.h"
#include <sstream>

class ParserTest : public ::testing::Test {
protected:
	std::vector<Parser::Type> types;

	void SetUp() override {
		types.push_back(Parser::Type::LALR1);
		types.push_back(Parser::Type::LR1);
	}
};

TEST_F(ParserTest, Basic) {
	for (Parser::Type const t : this->types) {
		Parser p;
		p.add_token("EQUALS", std::unique_ptr<RegexAST>(new RegexASTLiteral('=')));
		p.add_token("X", std::unique_ptr<RegexAST>(new RegexASTLiteral('x')));
		p.add_token("STAR", std::unique_ptr<RegexAST>(new RegexASTLiteral('*')));
		p.add_production("s", { "n" }, nullptr);
		p.add_production("n", { "v", "EQUALS", "e" }, nullptr);
		p.add_production("n", { "e" }, nullptr);
		p.add_production("e", { "v" }, nullptr);
		p.add_production("v", { "X" }, nullptr);
		p.add_production("v", { "STAR", "e" }, nullptr);
		p.generate(t, "s");
		ASSERT_EQ(p.conflicts().size(), 0);
		std::stringstream ss;
		ss << "*x=x";
		FileInputStream fis(&ss);
		ASSERT_NE(p.parse(&fis), nullptr);
	}
}

TEST_F(ParserTest, Recursion) {
	for (Parser::Type const t : this->types) {
		Parser p;
		p.add_token("A", std::unique_ptr<RegexAST>(new RegexASTLiteral('a')));
		p.add_production("n", { "n", "n" }, nullptr);
		p.add_production("n", { "A" }, nullptr);
		p.generate(t, "n");
		ASSERT_EQ(p.conflicts().size(), 1);
		std::stringstream ss;
		ss << "aaa";
		FileInputStream fis(&ss);
		ASSERT_NE(p.parse(&fis), nullptr);
	}
}

TEST_F(ParserTest, Epsilon) {
	for (Parser::Type const t : this->types) {
		Parser p;
		p.add_token("A", std::unique_ptr<RegexAST>(new RegexASTLiteral('a')));
		p.add_production("n", { "m", "n" }, nullptr);
		p.add_production("n", {}, nullptr);
		p.add_production("m", { "A" }, nullptr);
		p.generate(t, "n");
		ASSERT_EQ(p.conflicts().size(), 0);
		std::stringstream ss;
		ss << "aaa";
		FileInputStream fis(&ss);
		ASSERT_NE(p.parse(&fis), nullptr);
		p.reset();
		ss << "";
		ASSERT_NE(p.parse(&fis), nullptr);
	}
}

TEST_F(ParserTest, LALR1) {
	for (Parser::Type const t : this->types) {
		Parser p;
		p.add_token("EQUALS", std::unique_ptr<RegexAST>(new RegexASTLiteral('=')));
		p.add_token("STAR", std::unique_ptr<RegexAST>(new RegexASTLiteral('*')));
		p.add_token("ID", std::unique_ptr<RegexAST>(new RegexASTLiteral('i')));
		p.add_production("s", { "l", "EQUALS", "r" }, nullptr);
		p.add_production("s", { "r" }, nullptr);
		p.add_production("l", { "STAR", "r" }, nullptr);
		p.add_production("l", { "ID" }, nullptr);
		p.add_production("r", { "l" }, nullptr);
		p.generate(Parser::Type::LALR1, "s");
		std::stringstream ss;
		ss << "*i=i";
		FileInputStream fis(&ss);
		ASSERT_NE(p.parse(&fis), nullptr);
	}
}

TEST_F(ParserTest, Another) {
	auto prepare = [](Parser* p) -> void {
		p->add_token("C", std::unique_ptr<RegexAST>(new RegexASTLiteral('c')));
		p->add_token("D", std::unique_ptr<RegexAST>(new RegexASTLiteral('d')));
		p->add_production("s", { "c", "c" }, nullptr);
		p->add_production("c", { "C", "c" }, nullptr);
		p->add_production("c", { "D" }, nullptr);
	};
	{
		Parser p;
		prepare(&p);
		p.generate(Parser::Type::LALR1, "s");
		ASSERT_EQ(p.conflicts().size(), 0);
		std::stringstream ss;
		ss << "ccdccd";
		FileInputStream fis(&ss);
		ASSERT_NE(p.parse(&fis), nullptr);
	}
	{
		Parser p;
		prepare(&p);
		p.generate(Parser::Type::LR1, "s");
		ASSERT_EQ(p.conflicts().size(), 0);
		std::stringstream ss;
		ss << "ccdccd";
		FileInputStream fis(&ss);
		ASSERT_NE(p.parse(&fis), nullptr);
	}
}

TEST_F(ParserTest, LR1) {
	auto prepare = [](Parser* p) -> void {
		p->add_token("A", std::unique_ptr<RegexAST>(new RegexASTLiteral('a')));
		p->add_token("B", std::unique_ptr<RegexAST>(new RegexASTLiteral('b')));
		p->add_token("E", std::unique_ptr<RegexAST>(new RegexASTLiteral('e')));
		p->add_production("s", { "A", "e", "A" }, nullptr);
		p->add_production("s", { "B", "e", "B" }, nullptr);
		p->add_production("s", { "A", "f", "B" }, nullptr);
		p->add_production("s", { "B", "f", "A" }, nullptr);
		p->add_production("e", { "E" }, nullptr);
		p->add_production("f", { "E" }, nullptr);
	};
	{
		Parser p;
		prepare(&p);
		p.generate(Parser::Type::LALR1, "s");
		ASSERT_EQ(p.conflicts().size(), 2);
		std::stringstream ss;
		ss << "aea";
		FileInputStream fis(&ss);
		ASSERT_NE(p.parse(&fis), nullptr);
		ss.str("");
		ss.clear();
		ss << "afa";
		ASSERT_EQ(p.parse(&fis), nullptr);
	}
	{
		Parser p;
		prepare(&p);
		p.generate(Parser::Type::LR1, "s");
		ASSERT_EQ(p.conflicts().size(), 0);
		std::stringstream ss;
		ss << "aea";
		FileInputStream fis(&ss);
		ASSERT_NE(p.parse(&fis), nullptr);
	}
}

TEST_F(ParserTest, RegexGroup) {
	for (Parser::Type const t : this->types) {
		Parser p;
		p.add_token("LB", std::unique_ptr<RegexAST>(new RegexASTLiteral('[')));
		p.add_token("RB", std::unique_ptr<RegexAST>(new RegexASTLiteral(']')));
		p.add_token("DASH", std::unique_ptr<RegexAST>(new RegexASTLiteral('-')));
		p.add_token("ANY", std::unique_ptr<RegexAST>(RegexASTGroup::make(true, { '[', '[', ']', ']' })));
		p.add_production("class", { "LB", "class_contents", "RB" }, nullptr);
		p.add_production("class_contents", { "class_element" }, nullptr);
		p.add_production("class_contents", { "class_element", "class_contents" }, nullptr);
		p.add_production("class_element", { "literal" }, nullptr);
		p.add_production("class_element", { "class_element", "DASH", "literal" }, nullptr);
		p.add_production("literal", { "DASH" }, nullptr);
		p.add_production("literal", { "ANY" }, nullptr);
		p.generate(t, "class");
		std::stringstream ss;
		ss << "[-a-c-d-]";
		FileInputStream fis(&ss);
		ASSERT_NE(p.parse(&fis), nullptr);
	}
}
