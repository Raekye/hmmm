#include <sstream>
#include <fstream>
#include "helper.h"
#include "regex.h"
#include "lexer.h"
#include "parser.h"
#include "generator.h"

int test_parser() {
	std::unique_ptr<Parser> p = RegexParserGenerator::make();

	std::stringstream ss;
	ss << "abc{3,4}|def[ghi]+";
	std::unique_ptr<Match> m = p->parse(&ss);
	MatchedNonterminal* n = dynamic_cast<MatchedNonterminal*>(m.get());
	ParserRegexAST* r = dynamic_cast<ParserRegexAST*>(n->value.get());
	RegexASTPrinter printer;
	r->regex->accept(&printer);
	return 0;
}

int test_lexer() {
	Lexer l;
	l.add_rule(Rule("STAR", ""), std::unique_ptr<RegexAST>(new RegexASTLiteral('*')));
	l.add_rule(Rule("PLUS", ""), std::unique_ptr<RegexAST>(new RegexASTLiteral('+')));
	l.add_rule(Rule("QUESTION", ""), std::unique_ptr<RegexAST>(new RegexASTLiteral('?')));
	l.add_rule(Rule("OR", ""), std::unique_ptr<RegexAST>(new RegexASTLiteral('|')));
	l.add_rule(Rule("ESCAPE", ""), std::unique_ptr<RegexAST>(new RegexASTLiteral('\\')));
	l.add_rule(Rule("DOT", ""), std::unique_ptr<RegexAST>(new RegexASTLiteral('.')));

	l.add_rule(Rule("LPAREN", ""), std::unique_ptr<RegexAST>(new RegexASTLiteral('(')));
	l.add_rule(Rule("RPAREN", ""), std::unique_ptr<RegexAST>(new RegexASTLiteral(')')));
	l.add_rule(Rule("LBRACE", ""), std::unique_ptr<RegexAST>(new RegexASTLiteral('{')));
	l.add_rule(Rule("RBRACE", ""), std::unique_ptr<RegexAST>(new RegexASTLiteral('}')));
	l.add_rule(Rule("LBRACKET", ""), std::unique_ptr<RegexAST>(new RegexASTLiteral('[')));
	l.add_rule(Rule("RBRACKET", ""), std::unique_ptr<RegexAST>(new RegexASTLiteral(']')));

	l.add_rule(Rule("DASH", ""), std::unique_ptr<RegexAST>(new RegexASTLiteral('-')));
	l.add_rule(Rule("COMMA", ""), std::unique_ptr<RegexAST>(new RegexASTLiteral(',')));

	l.add_rule(Rule("X", ""), std::unique_ptr<RegexAST>(new RegexASTLiteral('x')));
	l.add_rule(Rule("T", ""), std::unique_ptr<RegexAST>(new RegexASTLiteral('t')));
	l.add_rule(Rule("N", ""), std::unique_ptr<RegexAST>(new RegexASTLiteral('n')));

	l.add_rule(Rule("ANY", ""), std::unique_ptr<RegexAST>(new RegexASTWildcard));

	std::stringstream ss;
	ss << "abcghi ";
	ss << "abcdxyz ";
	ss << "var x = 30; ";
	std::unique_ptr<Token> t = nullptr;
	while ((t = l.scan(&ss))) {
		mdk::printf("Read token tag %s, lexeme '%s'\n", t->tag.c_str(), t->lexeme.c_str());
	}
	mdk::print("Done.\n");
	return 0;
}

/*
int test_parser() {
	std::string indents = "";
	ProductionHandler fn = [&indents, &fn](Match* m) -> void {
		std::stack<Match*> s;
		s.push(m);
		while (s.size() > 0) {
			m = s.top();
			s.pop();
			if (MatchedTerminal* mt = dynamic_cast<MatchedTerminal*>(m)) {
				mdk::logf("%s- terminal %s, %s\n", indents.c_str(), mt->token->tag.c_str(), mt->token->lexeme.c_str());
			} else if (MatchedNonterminal* mnt = dynamic_cast<MatchedNonterminal*>(m)) {
				mdk::logf("%s- nonterminal ", indents.c_str());
				Parser::debug_production(mnt->production);
				indents += "  ";
				for (std::unique_ptr<Match>& x : mnt->terms) {
					fn(x.get());
				}
				indents = indents.substr(0, indents.length() - 2);
			}
		}
	};
	Parser parser;
	parser.set_start("s");
	parser.add_token("STAR", "\\*");
	parser.add_token("X", "x");
	parser.add_token("EQUALS", "=");
	parser.add_production("s", { "n" }, fn);
	parser.add_production("n", { "v", "EQUALS", "e" }, fn);
	parser.add_production("n", { "e" }, fn);
	parser.add_production("e", { "v" }, fn);
	parser.add_production("v", { "X" }, fn);
	parser.add_production("v", { "STAR", "e" }, fn);
	std::stringstream ss;
	ss << "x=*x";
	parser.parse(&ss);
	return 0;
}

int test_generator() {
	std::fstream f("src/parser.txt", std::fstream::in);
	std::unique_ptr<Parser> p = Parser::from_file(&f);
	(void) p;
	return 0;
}
*/

int main() {
	test_lexer();
	test_parser();
	//test_generator();
	return 0;
}
