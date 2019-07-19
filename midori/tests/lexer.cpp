#include "gtest/gtest.h"
#include "midori/lexer.h"
#include <sstream>

class LexerTest : public ::testing::Test {
};

TEST_F(LexerTest, Hmmm) {
	std::unique_ptr<RegexASTGroup> n1(new RegexASTGroup(false, nullptr));
	n1->add_range('a', 'c');
	n1->add_range('b', 'd');
	std::unique_ptr<RegexASTGroup> n2(new RegexASTGroup(false, nullptr));
	n2->add_range('o', 'q');
	n2->add_range('n', 'p');
	Lexer l;
	l.add_rule(Rule("hello", ""), std::unique_ptr<RegexAST>(new RegexASTMultiplication(std::move(n1), 0, 0)));
	l.add_rule(Rule("world", ""), std::unique_ptr<RegexAST>(new RegexASTMultiplication(std::move(n2), 1, 0)));
	l.generate();
	std::stringstream ss;
	ss << "abcdnopqabcdnopq";
	std::unique_ptr<Token> t = l.scan(&ss);
	ASSERT_EQ(t->tag, "hello");
	t = l.scan(&ss);
	ASSERT_EQ(t->tag, "world");
	t = l.scan(&ss);
	ASSERT_EQ(t->tag, "hello");
	t = l.scan(&ss);
	ASSERT_EQ(t->tag, "world");
}
