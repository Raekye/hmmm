#include "gtest/gtest.h"
#include "midori/lexer.h"
#include <sstream>
#include "midori/regex_engine.h"

class RegexEngineTest : public ::testing::Test {
};

TEST_F(RegexEngineTest, Dash) {
	RegexEngine re;
	Lexer l;
	l.add_rule("a", re.parse("[-b-a-cd---]"));
	l.generate();
	std::stringstream ss;
	for (char ch = '-'; ch <= 'd'; ch++) {
		ss << ch;
	}
	FileInputStream fis(&ss);
	for (char ch = '-'; ch <= 'd'; ch++) {
		std::unique_ptr<Token> t = l.scan(&fis);
		ASSERT_EQ(t->tags.at(0), "a");
	}
	ss.clear();
	ss << (char) ('-' - 1);
	std::unique_ptr<Token> t = l.scan(&fis);
	ASSERT_EQ(t->tags.at(0), Lexer::TOKEN_BAD);
	ASSERT_EQ(t->lexeme, std::string(1, ('-' - 1)));
	l.reset();
	ss.clear();
	ss << (char) ('d' + 1);
	t = l.scan(&fis);
	ASSERT_EQ(t->tags.at(0), Lexer::TOKEN_BAD);
	ASSERT_EQ(t->lexeme, std::string(1, ('d' + 1)));
}
