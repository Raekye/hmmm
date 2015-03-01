#include <iostream>
#include <sstream>
#include "regex.h"
#include "lexer.h"

int main() {
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
	std::stringstream ss;
	ss << "a";
	Token* t = l.scan(&ss);
	if (t) {
		std::cout << "Token was good" << std::endl;
		std::cout << t->lexeme << std::endl;
	} else {
		std::cout << "Token was null" << std::endl;
	}
	return 0;
}
