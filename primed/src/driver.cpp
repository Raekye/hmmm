#include <iostream>
#include "regex.h"
#include "lexer.h"
#include <sstream>

int main() {
	Lexer l;
	l.add_rule(Rule("rule1", "abc[def]*", "tag1"));
	std::stringstream ss;
	ss << "abcfefefee";
	Token* t = l.scan(&ss);
	l.print_states();
	if (t) {
		std::cout << "Token was good" << std::endl;
		std::cout << t->lexeme << std::endl;
	} else {
		std::cout << "Token was null" << std::endl;
	}
	return 0;
}
