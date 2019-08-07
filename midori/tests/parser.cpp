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

TEST_F(ParserTest, Precedence) {
	auto prepare = [](Parser* p) -> void {
		p->add_token("a", std::unique_ptr<RegexAST>(new RegexASTLiteral('a')));
		p->add_token("plus", std::unique_ptr<RegexAST>(new RegexASTLiteral('+')));
		p->add_token("minus", std::unique_ptr<RegexAST>(new RegexASTLiteral('-')));
		p->set_precedence("plus", "precedence");
		p->set_precedence("minus", "precedence");
		p->add_production("expr", { "expr", "plus", "expr" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
			Int x = m->nonterminal(0)->value->get<Int>();
			Int y = m->nonterminal(2)->value->get<Int>();
			std::cout << "x " << x << ", y " << y << std::endl;
			return std::unique_ptr<ParserAST>(new ParserValue<Int>(x + y));
		});
		p->add_production("expr", { "expr", "minus", "expr" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
			Int x = m->nonterminal(0)->value->get<Int>();
			Int y = m->nonterminal(2)->value->get<Int>();
			return std::unique_ptr<ParserAST>(new ParserValue<Int>(x - y));
		});
		p->add_production("expr", { "a" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
			return std::unique_ptr<ParserAST>(new ParserValue<Int>(1));
		});
	};
	for (Parser::Type const t : this->types) {
		Parser p;
		prepare(&p);
		p.generate(t, "expr");
		ASSERT_EQ(p.conflicts().size(), 4);
		std::stringstream ss;
		ss << "a-a+a";
		FileInputStream fis(&ss);
		std::unique_ptr<MatchedNonterminal> m = p.parse(&fis);
		Int z = m->value->get<Int>();
		ASSERT_EQ(z, -1);
	}
	for (Parser::Type const t : this->types) {
		Parser p;
		p.set_precedence_class("precedence", 1, Precedence::Associativity::LEFT);
		prepare(&p);
		p.generate(t, "expr");
		ASSERT_EQ(p.conflicts().size(), 0);
		std::stringstream ss;
		ss << "a-a+a";
		FileInputStream fis(&ss);
		std::unique_ptr<MatchedNonterminal> m = p.parse(&fis);
		Int z = m->value->get<Int>();
		ASSERT_EQ(z, 1);
	}
	for (Parser::Type const t : this->types) {
		Parser p;
		p.set_precedence_class("precedence", 1, Precedence::Associativity::RIGHT);
		prepare(&p);
		p.generate(t, "expr");
		ASSERT_EQ(p.conflicts().size(), 0);
		std::stringstream ss;
		ss << "a-a+a";
		FileInputStream fis(&ss);
		std::unique_ptr<MatchedNonterminal> m = p.parse(&fis);
		Int z = m->value->get<Int>();
		ASSERT_EQ(z, -1);
	}
	for (Parser::Type const t : this->types) {
		Parser p;
		p.set_precedence_class("precedence", 1, Precedence::Associativity::NONE);
		p.set_precedence_class("precedence", 1, Precedence::Associativity::NONASSOC);
		prepare(&p);
		p.generate(t, "expr");
		ASSERT_EQ(p.conflicts().size(), 0);
		std::stringstream ss;
		ss << "a-a+a";
		FileInputStream fis(&ss);
		std::unique_ptr<MatchedNonterminal> m = p.parse(&fis);
		ASSERT_EQ(m, nullptr);
	}
}
