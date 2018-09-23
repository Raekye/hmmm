#include <sstream>
#include <fstream>
#include "helper.h"
#include "regex.h"
#include "lexer.h"
#include "parser.h"

int test_parser() {
	Parser p;
	p.add_token("STAR", "", std::unique_ptr<RegexAST>(new RegexASTLiteral('*')));
	p.add_token("PLUS", "", std::unique_ptr<RegexAST>(new RegexASTLiteral('+')));
	p.add_token("QUESTION", "", std::unique_ptr<RegexAST>(new RegexASTLiteral('?')));
	p.add_token("OR", "", std::unique_ptr<RegexAST>(new RegexASTLiteral('|')));
	p.add_token("ESCAPE", "", std::unique_ptr<RegexAST>(new RegexASTLiteral('\\')));
	p.add_token("DOT", "", std::unique_ptr<RegexAST>(new RegexASTLiteral('.')));

	p.add_token("LPAREN", "", std::unique_ptr<RegexAST>(new RegexASTLiteral('(')));
	p.add_token("RPAREN", "", std::unique_ptr<RegexAST>(new RegexASTLiteral(')')));
	p.add_token("LBRACE", "", std::unique_ptr<RegexAST>(new RegexASTLiteral('{')));
	p.add_token("RBRACE", "", std::unique_ptr<RegexAST>(new RegexASTLiteral('}')));
	p.add_token("LBRACKET", "", std::unique_ptr<RegexAST>(new RegexASTLiteral('[')));
	p.add_token("RBRACKET", "", std::unique_ptr<RegexAST>(new RegexASTLiteral(']')));

	p.add_token("DASH", "", std::unique_ptr<RegexAST>(new RegexASTLiteral('-')));
	p.add_token("COMMA", "", std::unique_ptr<RegexAST>(new RegexASTLiteral(',')));

	p.add_token("X", "", std::unique_ptr<RegexAST>(new RegexASTLiteral('x')));
	p.add_token("T", "", std::unique_ptr<RegexAST>(new RegexASTLiteral('t')));
	p.add_token("N", "", std::unique_ptr<RegexAST>(new RegexASTLiteral('n')));

	p.add_token("ANY", "", std::unique_ptr<RegexAST>(new RegexASTWildcard));

	ProductionHandler fn = [](Match* m) -> void {
		Parser::debug_match(m, 0);
	};

	p.set_start("regex");
	p.add_production("regex", { "lr_or" }, fn);
	p.add_production("lr_or", { "lr_add", "OR", "lr_or" }, fn);
	p.add_production("lr_or", { "lr_add" }, fn);
	p.add_production("lr_add", { "lr_mul", "lr_add" }, fn);
	p.add_production("lr_add", { "lr_mul" }, fn);

	p.add_production("lr_mul", { "not_lr", "STAR" }, fn);
	p.add_production("lr_mul", { "not_lr", "PLUS" }, fn);
	p.add_production("lr_mul", { "not_lr", "QUESTION" }, fn);
	p.add_production("lr_mul", { "not_lr", "mul_range" }, fn);
	p.add_production("lr_mul", { "not_lr" }, fn);

	p.add_production("not_lr", { "parentheses" }, fn);
	p.add_production("not_lr", { "literal" }, fn);
	p.add_production("not_lr", { "group" }, fn);

	p.add_production("parentheses", { "LPAREN", "lr_or", "RPAREN" }, fn);

	p.add_production("literal", { "escaped_literal" }, fn);
	p.add_production("literal", { "ANY" }, fn);

	p.add_production("group", { "LBRACKET", "group_contents", "RBRACKET" }, fn);
	p.add_production("group_contents", { "group_element", "group_contents" }, fn);
	p.add_production("group_contents", { "group_element" }, fn);
	p.add_production("group_element", { "group_range" }, fn);
	p.add_production("group_element", { "literal" }, fn);
	p.add_production("group_range", { "literal", "DASH", "literal" }, fn);

	p.add_production("escaped_literal", { "ESCAPE", "escape_code" }, fn);
	p.add_production("escape_code", { "ANY", "escape_code" }, fn);
	p.add_production("escape_code", { "ANY" }, fn);

	p.add_production("mul_range", { "LBRACE", "dec_int", "COMMA", "dec_int", "RBRACE" }, fn);
	p.add_production("dec_int", { "ANY", "dec_int" }, fn);
	p.add_production("dec_int", { "ANY" }, fn);

	std::stringstream ss;
	ss << "abc|def[ghi]";
	p.parse(&ss);
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
