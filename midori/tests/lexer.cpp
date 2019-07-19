#include "gtest/gtest.h"
#include "midori/lexer.h"
#include <sstream>
#include <vector>

class VectorInputStream : public IInputStream {
public:
	VectorInputStream(std::vector<UInt> v) : v(v), pos(0) {
		return;
	}

	Long get() override {
		if (this->pos >= this->v.size()) {
			return -1;
		}
		return this->v.at(this->pos++);
	}

private:
	std::vector<UInt> v;
	size_t pos;
};

class LexerTest : public ::testing::Test {
};

TEST_F(LexerTest, Ascii) {
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
	ASSERT_EQ(t->tag, "hello");
	ASSERT_EQ(t->lexeme, "abcd");
	t = l.scan(&fis);
	ASSERT_EQ(t->tag, "world");
	ASSERT_EQ(t->lexeme, "nopq");
	t = l.scan(&fis);
	ASSERT_EQ(t->tag, "hello");
	ASSERT_EQ(t->lexeme, "abcd");
	t = l.scan(&fis);
	ASSERT_EQ(t->tag, "world");
	ASSERT_EQ(t->lexeme, "nopq");
}

TEST_F(LexerTest, Ranges) {
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
	ASSERT_EQ(t->tag, "hello");
	ASSERT_EQ(t->lexeme.length(), 6);
	t = l.scan(&vis);
	ASSERT_EQ(t->tag, "world");
	ASSERT_EQ(t->lexeme.length(), 6);
}
