#include "gtest/gtest.h"
#include "midori/lexer.h"
#include <sstream>
#include "midori/regex_engine.h"

TEST(RegexEngineTest, Dash) {
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

TEST(RegexEngineTest, GroupNegation) {
	RegexEngine re;
	std::unique_ptr<RegexAST> r = re.parse("[^a-c]");
	RegexASTPrinter p;
	std::cout << "=== Here" << std::endl;
	r->accept(&p);
	std::cout << "=== End here" << std::endl;
	Lexer l;
	l.add_rule("a", re.parse("[^a-c]"));
	l.generate();
	for (char ch = 'a'; ch <= 'c'; ch++) {
		l.reset();
		std::stringstream ss;
		ss << std::string(1, ch);
		FileInputStream fis(&ss);
		std::unique_ptr<Token> t = l.scan(&fis);
		ASSERT_EQ(t->tags.at(0), Lexer::TOKEN_BAD);
	}
	l.reset();
	std::stringstream ss;
	ss << "d";
	FileInputStream fis(&ss);
	std::unique_ptr<Token> t = l.scan(&fis);
	ASSERT_EQ(t->tags.at(0), "a");
	ASSERT_EQ(t->lexeme, "d");
}

TEST(RegexEngineTest, Rewrite) {
	RegexEngine re;
	Lexer l;
	l.add_rule("a", re.parse("a{3}"));
	l.generate();
	for (Int i = 0; i <= 3; i++) {
		l.reset();
		std::stringstream ss;
		std::string str(i, 'a');
		ss << str;
		FileInputStream fis(&ss);
		std::unique_ptr<Token> t = l.scan(&fis);
		ASSERT_EQ(t->tags.at(0), "a");
		ASSERT_EQ(t->lexeme, str);
		t = l.scan(&fis);
		ASSERT_EQ(t->tags.at(0), "a");
		ASSERT_EQ(t->lexeme, "");
	}
}
