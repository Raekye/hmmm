#include <iostream>
#include "regex.h"
#include "lexer.h"
#include <sstream>

int main() {
	RegexParser p;
	RegexAST* r = p.parse("[a-e]{2,5}");
	r->mark_terminal();
	RegexASTPrinter rp;
	r->accept(&rp);
	std::cout << "###" << std::endl;
	RegexNFAGenerator rng;
	r->accept(&rng);
	for (UInt i = 0; i < rng.nfa.states.size(); i++) {
		std::cout << "State " << i;
		if (rng.nfa.states[i]->terminal) {
			std::cout << " terminal";
		}
		std::cout << std::endl;
		for (auto it = rng.nfa.states[i]->next_states.begin(); it != rng.nfa.states[i]->next_states.end(); it++) {
			std::cout << "\tChar " << (char) it->first << ":";
			for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
				std::cout << " " << (*it2)->id << ",";
			}
			std::cout << std::endl;
		}
	}
	return 0;
	Lexer l;
	l.add_rule(Rule("rule1", "a(xyzb)*c|def", "tag1"));
	std::stringstream ss;
	ss << "a";
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
