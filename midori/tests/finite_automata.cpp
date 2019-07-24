#include "gtest/gtest.h"
#include "midori/regex_ast.h"

TEST(FiniteAutomataTest, Main) {
	std::unique_ptr<RegexAST> n1(new RegexASTMultiplication(std::unique_ptr<RegexAST>(new RegexASTLiteral('a')), 0, 0));
	std::unique_ptr<RegexAST> n2(new RegexASTMultiplication(std::unique_ptr<RegexAST>(new RegexASTLiteral('b')), 0, 0));
	std::unique_ptr<RegexAST> n3(new RegexASTOr(std::move(n1), std::move(n2)));
	RegexNFAGenerator g;
	g.new_rule("test");
	n3->accept(&g);
}
