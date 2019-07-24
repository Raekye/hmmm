#include "gtest/gtest.h"
#include "midori/lexer.h"
#include <sstream>
#include <vector>
#include "midori/regex_engine.h"

TEST(LexerTest, Ascii) {
	std::unique_ptr<RegexASTGroup> n1(new RegexASTGroup(false, nullptr));
	n1->add_range('a', 'c');
	n1->add_range('b', 'd');
	std::unique_ptr<RegexASTGroup> n2(new RegexASTGroup(false, nullptr));
	n2->add_range('o', 'q');
	n2->add_range('n', 'p');
	Lexer l;
	l.add_rule("hello", std::unique_ptr<RegexAST>(new RegexASTMultiplication(std::move(n1), 0, 0)));
	l.add_rule("world", std::unique_ptr<RegexAST>(new RegexASTMultiplication(std::move(n2), 1, 0)));
	l.generate();
	std::stringstream ss;
	ss << "abcdnopqabcdnopq";
	FileInputStream fis(&ss);
	std::unique_ptr<Token> t = l.scan(&fis);
	ASSERT_EQ(t->tags.at(0), "hello");
	ASSERT_EQ(t->lexeme, "abcd");
	t = l.scan(&fis);
	ASSERT_EQ(t->tags.at(0), "world");
	ASSERT_EQ(t->lexeme, "nopq");
	t = l.scan(&fis);
	ASSERT_EQ(t->tags.at(0), "hello");
	ASSERT_EQ(t->lexeme, "abcd");
	t = l.scan(&fis);
	ASSERT_EQ(t->tags.at(0), "world");
	ASSERT_EQ(t->lexeme, "nopq");
}

TEST(LexerTest, Ranges) {
	std::unique_ptr<RegexASTGroup> n1(new RegexASTGroup(false, nullptr));
	n1->add_range(200, 299);
	n1->add_range(400, 499);
	std::unique_ptr<RegexASTGroup> n2(new RegexASTGroup(false, nullptr));
	n2->add_range(300, 399);
	n2->add_range(500, 599);
	Lexer l;
	l.add_rule("hello", std::unique_ptr<RegexAST>(new RegexASTMultiplication(std::move(n1), 0, 0)));
	l.add_rule("world", std::unique_ptr<RegexAST>(new RegexASTMultiplication(std::move(n2), 1, 0)));
	l.generate();
	VectorInputStream vis({ 200, 201, 202, 300, 301, 303 });
	std::unique_ptr<Token> t = l.scan(&vis);
	ASSERT_EQ(t->tags.at(0), "hello");
	ASSERT_EQ(t->lexeme.length(), 6);
	t = l.scan(&vis);
	ASSERT_EQ(t->tags.at(0), "world");
	ASSERT_EQ(t->lexeme.length(), 6);
}

TEST(LexerTest, Multiplication) {
	RegexEngine re;
	Lexer l;
	l.add_rule("a", re.parse("(abc)?(def|ghi)*"));
	l.add_rule("b", re.parse("(abc)+xyz"));
	l.generate();
	std::stringstream ss;
	ss << "ghi";
	ss << "abc";
	ss << "abcdefghidef";
	ss << "abcxyz";
	FileInputStream fis(&ss);

	std::unique_ptr<Token> t = l.scan(&fis);
	ASSERT_EQ(t->tags.at(0), "a");
	ASSERT_EQ(t->lexeme, "ghi");

	t = l.scan(&fis);
	ASSERT_EQ(t->tags.at(0), "a");
	ASSERT_EQ(t->lexeme, "abc");

	t = l.scan(&fis);
	ASSERT_EQ(t->tags.at(0), "a");
	ASSERT_EQ(t->lexeme, "abcdefghidef");

	t = l.scan(&fis);
	ASSERT_EQ(t->tags.at(0), "b");
	ASSERT_EQ(t->lexeme, "abcxyz");
}

TEST(LexerTest, LocationInfo) {
	Lexer l;
	std::string str;
	for (char ch = 'a'; ch <= 'z'; ch++) {
		l.add_rule(std::string(1, (ch - 'a' + 'A')), std::unique_ptr<RegexAST>(new RegexASTLiteral(ch)));
		str.append(std::string(1, ch));
	}
	l.add_rule("NL", std::unique_ptr<RegexAST>(new RegexASTLiteral('\n')));
	l.generate();
	std::stringstream ss;
	for (Int i = 0; i < 4; i++) {
		ss << str << "\n";
	}
	FileInputStream fis(&ss);
	for (Int i = 0; i < 4; i++) {
		for (Int j = 0; j < 26; j++) {
			std::unique_ptr<Token> t = l.scan(&fis);
			ASSERT_EQ(t->loc.line, i + 1);
			ASSERT_EQ(t->loc.column, j + 1);
		}
		std::unique_ptr<Token> t = l.scan(&fis);
		ASSERT_EQ(t->loc.line, i + 1);
		ASSERT_EQ(t->loc.column, 27);
	}
	std::unique_ptr<Token> t = l.scan(&fis);
	ASSERT_EQ(t->tags.at(0), Lexer::TOKEN_END);
	ASSERT_EQ(t->loc.line, 5);
	ASSERT_EQ(t->loc.column, 1);
}
