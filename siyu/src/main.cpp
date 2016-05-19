#include <iostream>
#include <cstdio>
#include <sstream>
#include "regex.h"
#include "lexer.h"
#include "parser.h"

int test_lexer() {
	//RegexAST* r = p.parse("[a-e]{2,4}z(yx|wv)*123{0,2}4{2,0}");
	// repetition tests
	//RegexAST* r = p.parse("[a-e]{2,5}");
	//RegexAST* r = p.parse("[a-e]{0,5}");
	//RegexAST* r = p.parse("[a-e]{5,0}");
	//RegexAST* r = p.parse("[a-e]{0,0}");
	Lexer l;
	l.add_rule(Rule("tag1", "abcdef|abcghi"));
	l.add_rule(Rule("tag2", "abcdxyz"));
	l.add_rule(Rule("tag3", "[m-o]{3,0}"));
	l.add_rule(Rule(":whitespace", "[ \\t]+"));
	l.add_rule(Rule(":var", "var"));
	l.add_rule(Rule(":identifier", "[a-z][a-zA-Z0-9_]*"));
	//l.add_rule(Rule(":identifier", "[a-z]+"));
	l.add_rule(Rule(":typename", "[A-Z][a-zA-Z0-9_]*"));
	l.add_rule(Rule(":equals", "="));
	l.add_rule(Rule(":semicolon", ";"));
	l.add_rule(Rule(":integer_literal", "[0-9]+"));
	std::stringstream ss;
	ss << "abcghi ";
	ss << "abcdxyz ";
	ss << "var x = 30; ";
	Token* t = nullptr;
	while ((t = l.scan(&ss))) {
		printf("Read token tag %s, lexeme '%s'\n", t->tag.c_str(), t->lexeme.c_str());
	}
	printf("Done.\n");
	return 0;
}

int test_parser() {
	Parser parser;
	parser.set_start("s");
	parser.add_token("STAR", "\\*");
	parser.add_token("X", "x");
	parser.add_token("EQUALS", "=");
	parser.add_production("s", { "n" }, nullptr);
	parser.add_production("n", { "v", "EQUALS", "e" }, nullptr);
	parser.add_production("n", { "e" }, nullptr);
	parser.add_production("e", { "v" }, nullptr);
	parser.add_production("v", { "X" }, nullptr);
	parser.add_production("v", { "STAR", "e" }, nullptr);
	parser.parse(nullptr);
	return 0;
}

int main() {
	//return test_lexer();
	return test_parser();
}
