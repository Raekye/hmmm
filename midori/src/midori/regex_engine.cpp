#include "regex_engine.h"

#include "helper.h"
#include "utf8.h"
#include <cstdlib>
#include <sstream>
#include <algorithm>

class ParserASTRegex : public ParserAST {
public:
	std::unique_ptr<RegexAST> regex;
	ParserASTRegex(std::unique_ptr<RegexAST> r) : regex(std::move(r)) {
		return;
	}
};

class ParserASTRange : public ParserAST {
public:
	UInt min;
	UInt max;
	ParserASTRange(UInt a, UInt b) : min(a), max(b) {
		return;
	}
};

RegexEngine::RegexEngine() {
	this->parser = RegexEngine::make();
}

std::unique_ptr<RegexAST> RegexEngine::parse(std::string pattern) {
	std::stringstream ss;
	ss << pattern;
	FileInputStream fis(&ss);
	std::unique_ptr<Match> m = this->parser->parse(&fis);
	MatchedNonterminal* n = dynamic_cast<MatchedNonterminal*>(m.get());
	ParserASTRegex* r = dynamic_cast<ParserASTRegex*>(n->value.get());
	return std::move(r->regex);
}

std::unique_ptr<Parser> RegexEngine::make() {
	std::unique_ptr<Parser> p(new Parser);
	p->add_token("STAR", std::unique_ptr<RegexAST>(new RegexASTLiteral('*')));
	p->add_token("PLUS", std::unique_ptr<RegexAST>(new RegexASTLiteral('+')));
	p->add_token("QUESTION", std::unique_ptr<RegexAST>(new RegexASTLiteral('?')));
	p->add_token("OR", std::unique_ptr<RegexAST>(new RegexASTLiteral('|')));
	p->add_token("ESCAPE", std::unique_ptr<RegexAST>(new RegexASTLiteral('\\')));
	p->add_token("DOT", std::unique_ptr<RegexAST>(new RegexASTLiteral('.')));

	p->add_token("LPAREN", std::unique_ptr<RegexAST>(new RegexASTLiteral('(')));
	p->add_token("RPAREN", std::unique_ptr<RegexAST>(new RegexASTLiteral(')')));
	p->add_token("LBRACE", std::unique_ptr<RegexAST>(new RegexASTLiteral('{')));
	p->add_token("RBRACE", std::unique_ptr<RegexAST>(new RegexASTLiteral('}')));
	p->add_token("LBRACKET", std::unique_ptr<RegexAST>(new RegexASTLiteral('[')));
	p->add_token("RBRACKET", std::unique_ptr<RegexAST>(new RegexASTLiteral(']')));

	p->add_token("HAT", std::unique_ptr<RegexAST>(new RegexASTLiteral('^')));
	p->add_token("DASH", std::unique_ptr<RegexAST>(new RegexASTLiteral('-')));
	p->add_token("COMMA", std::unique_ptr<RegexAST>(new RegexASTLiteral(',')));

	p->add_token("X", std::unique_ptr<RegexAST>(new RegexASTLiteral('x')));
	p->add_token("U", std::unique_ptr<RegexAST>(new RegexASTLiteral('u')));
	p->add_token("T", std::unique_ptr<RegexAST>(new RegexASTLiteral('t')));
	p->add_token("N", std::unique_ptr<RegexAST>(new RegexASTLiteral('n')));

	p->add_token("DEC", std::unique_ptr<RegexAST>(RegexASTGroup::make(false, { '0', '9' })));
	p->add_token("HEX", std::unique_ptr<RegexAST>(RegexASTGroup::make(false, { '0', '9', 'a', 'f' })));

	p->add_token("GROUP_ANY", std::unique_ptr<RegexAST>(RegexASTGroup::make(true, { '[', '[', ']', ']' })));
	p->add_token("ANY", std::unique_ptr<RegexAST>(RegexASTGroup::make(false, { 0, RegexASTGroup::UNICODE_MAX })));

	p->add_production("regex", { "lr_or" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedNonterminal* n = m->nonterminal(0);
		return std::move(n->value);
	});
	p->add_production("lr_or", { "lr_add", "OR", "lr_or" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedNonterminal* n1 = m->nonterminal(0);
		MatchedNonterminal* n2 = m->nonterminal(2);
		ParserASTRegex* r1 = dynamic_cast<ParserASTRegex*>(n1->value.get());
		ParserASTRegex* r2 = dynamic_cast<ParserASTRegex*>(n2->value.get());
		return std::unique_ptr<ParserAST>(new ParserASTRegex(std::unique_ptr<RegexAST>(new RegexASTOr(std::move(r1->regex), std::move(r2->regex)))));
	});
	p->add_production("lr_or", { "lr_add" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedNonterminal* n = m->nonterminal(0);
		return std::move(n->value);
	});
	p->add_production("lr_add", { "lr_mul", "lr_add" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedNonterminal* n1 = m->nonterminal(0);
		MatchedNonterminal* n2 = m->nonterminal(1);
		ParserASTRegex* r1 = dynamic_cast<ParserASTRegex*>(n1->value.get());
		ParserASTRegex* r2 = dynamic_cast<ParserASTRegex*>(n2->value.get());
		std::vector<std::unique_ptr<RegexAST>> vec;
		vec.push_back(std::move(r1->regex));
		vec.push_back(std::move(r2->regex));
		std::unique_ptr<RegexAST> ret(new RegexASTChain(std::move(vec)));
		/*
		 * The following does not work
			std::unique_ptr<RegexAST> ret(new RegexASTChain({
				std::move(r1->regex),
				std::move(r2->regex),
			}));
		 */
		return std::unique_ptr<ParserAST>(new ParserASTRegex(std::move(ret)));
	});
	p->add_production("lr_add", { "lr_mul" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedNonterminal* n = m->nonterminal(0);
		return std::move(n->value);
	});

	p->add_production("lr_mul", { "not_lr", "STAR" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedNonterminal* n1 = m->nonterminal(0);
		ParserASTRegex* r = dynamic_cast<ParserASTRegex*>(n1->value.get());
		std::unique_ptr<RegexAST> ret(new RegexASTMultiplication(std::move(r->regex), 0, 0));
		return std::unique_ptr<ParserAST>(new ParserASTRegex(std::move(ret)));
	});
	p->add_production("lr_mul", { "not_lr", "PLUS" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedNonterminal* n1 = m->nonterminal(0);
		ParserASTRegex* r = dynamic_cast<ParserASTRegex*>(n1->value.get());
		std::unique_ptr<RegexAST> ret(new RegexASTMultiplication(std::move(r->regex), 1, 0));
		return std::unique_ptr<ParserAST>(new ParserASTRegex(std::move(ret)));
	});
	p->add_production("lr_mul", { "not_lr", "QUESTION" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedNonterminal* n1 = m->nonterminal(0);
		ParserASTRegex* r = dynamic_cast<ParserASTRegex*>(n1->value.get());
		std::unique_ptr<RegexAST> ret(new RegexASTMultiplication(std::move(r->regex), 0, 1));
		return std::unique_ptr<ParserAST>(new ParserASTRegex(std::move(ret)));
	});
	p->add_production("lr_mul", { "not_lr", "mul_range" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedNonterminal* n1 = m->nonterminal(0);
		MatchedNonterminal* n2 = m->nonterminal(1);
		ParserASTRegex* r = dynamic_cast<ParserASTRegex*>(n1->value.get());
		ParserASTRange* range = dynamic_cast<ParserASTRange*>(n2->value.get());
		std::unique_ptr<RegexAST> ret(new RegexASTMultiplication(std::move(r->regex), (UInt) range->min, (UInt) range->max));
		return std::unique_ptr<ParserAST>(new ParserASTRegex(std::move(ret)));
	});
	p->add_production("lr_mul", { "not_lr" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedNonterminal* n = m->nonterminal(0);
		return std::move(n->value);
	});

	p->add_production("not_lr", { "parentheses" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedNonterminal* n = m->nonterminal(0);
		return std::move(n->value);
	});
	p->add_production("not_lr", { "literal" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedNonterminal* n = m->nonterminal(0);
		return std::move(n->value);
	});
	p->add_production("not_lr", { "group" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedNonterminal* n = m->nonterminal(0);
		return std::move(n->value);
	});
	p->add_production("not_lr", { "DOT" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		(void) m;
		return std::unique_ptr<ParserAST>(new ParserASTRegex(std::unique_ptr<RegexAST>(RegexASTGroup::make(false, { 0, RegexASTGroup::UNICODE_MAX }))));
	});

	p->add_production("parentheses", { "LPAREN", "lr_or", "RPAREN" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedNonterminal* n = m->nonterminal(1);
		return std::move(n->value);
	});

	p->add_production("literal", { "escaped_literal" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedNonterminal* n = m->nonterminal(0);
		return std::move(n->value);
	});
	/*
	p->add_production("literal", { "DEC" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedTerminal* n = m->terminal(0);
		return std::unique_ptr<ParserAST>(new ParserASTRegex(std::unique_ptr<RegexAST>(new RegexASTLiteral(n->token->lexeme.at(0)))));
	});
	p->add_production("literal", { "HEX" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedTerminal* n = m->terminal(0);
		return std::unique_ptr<ParserAST>(new ParserASTRegex(std::unique_ptr<RegexAST>(new RegexASTLiteral(n->token->lexeme.at(0)))));
	});
	RegexEngine::add_literal(p.get(), "literal", "N", 'n');
	RegexEngine::add_literal(p.get(), "literal", "T", 't');
	RegexEngine::add_literal(p.get(), "literal", "X", 'x');
	RegexEngine::add_literal(p.get(), "literal", "DASH", '-');
	RegexEngine::add_literal(p.get(), "literal", "COMMA", ',');
	*/
	p->add_production("literal", { "ANY" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedTerminal* n = m->terminal(0);
		Long ch = utf8::codepoint_from_string(n->token->lexeme, 0, nullptr);
		assert(ch >= 0);
		return std::unique_ptr<ParserAST>(new ParserASTRegex(std::unique_ptr<RegexAST>(new RegexASTLiteral(ch))));
	});

	p->add_production("group", { "LBRACKET", "group_contents", "RBRACKET" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedNonterminal* n = m->nonterminal(1);
		return std::move(n->value);
	});
	p->add_production("group", { "LBRACKET", "HAT", "group_contents", "RBRACKET" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedNonterminal* n = m->nonterminal(2);
		ParserASTRegex* r = dynamic_cast<ParserASTRegex*>(n->value.get());
		RegexASTGroup* g = dynamic_cast<RegexASTGroup*>(r->regex.get());
		g->negate = true;
		return std::move(n->value);
	});
	p->add_production("group_contents", { "group_element", "group_contents" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedNonterminal* n1 = m->nonterminal(0);
		MatchedNonterminal* n2 = m->nonterminal(1);
		ParserASTRegex* r1 = dynamic_cast<ParserASTRegex*>(n1->value.get());
		ParserASTRegex* r2 = dynamic_cast<ParserASTRegex*>(n2->value.get());
		RegexASTGroup* g1 = dynamic_cast<RegexASTGroup*>(r1->regex.get());
		RegexASTGroup* g2 = dynamic_cast<RegexASTGroup*>(r2->regex.get());
		assert(g1->span->next == nullptr);
		std::unique_ptr<RegexASTGroup::RangeList> car(new RegexASTGroup::RangeList);
		car->range.first = g1->span->range.first;
		car->range.second = g1->span->range.second;
		car->next = std::move(g2->span);
		return std::unique_ptr<ParserAST>(new ParserASTRegex(std::unique_ptr<RegexAST>(new RegexASTGroup(false, std::move(car)))));
	});
	p->add_production("group_contents", { "group_element" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedNonterminal* n = m->nonterminal(0);
		return std::move(n->value);
	});

	/*
	p->add_production("group_element", { "group_range" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedNonterminal* n = m->nonterminal(0);
		return std::move(n->value);
	});
	*/
	p->add_production("group_element", { "group_element", "DASH", "group_literal" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedNonterminal* n1 = m->nonterminal(0);
		MatchedNonterminal* n2 = m->nonterminal(2);
		ParserASTRegex* r1 = dynamic_cast<ParserASTRegex*>(n1->value.get());
		ParserASTRegex* r2 = dynamic_cast<ParserASTRegex*>(n2->value.get());
		std::unique_ptr<RegexAST> p1(std::move(r1->regex));
		std::unique_ptr<RegexAST> p2(std::move(r2->regex));
		RegexASTGroup* g1 = dynamic_cast<RegexASTGroup*>(p1.get());
		RegexASTGroup* g2 = dynamic_cast<RegexASTGroup*>(p2.get());
		assert(g1->span->next == nullptr);
		assert(g2->span->next == nullptr);
		assert(g2->span->range.first == g2->span->range.second);
		UInt a = g1->span->range.first;
		UInt b = g1->span->range.second;
		UInt c = g2->span->range.first;
		UInt lower = std::min({ a, b, c });
		UInt upper = std::max({ a, b, c });
		return std::unique_ptr<ParserAST>(new ParserASTRegex(std::unique_ptr<RegexAST>(RegexASTGroup::make(false, { lower, upper }))));
	});
	p->add_production("group_element", { "group_literal" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedNonterminal* n = m->nonterminal(0);
		return std::move(n->value);
	});

	p->add_production("group_literal", { "group_literal_char" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedNonterminal* n = m->nonterminal(0);
		ParserASTRegex* r = dynamic_cast<ParserASTRegex*>(n->value.get());
		RegexASTLiteral* l = dynamic_cast<RegexASTLiteral*>(r->regex.get());
		return std::unique_ptr<ParserAST>(new ParserASTRegex(std::unique_ptr<RegexAST>(RegexASTGroup::make(false, { l->ch, l->ch }))));
	});
	p->add_production("group_literal_char", { "group_escaped_literal" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedNonterminal* n = m->nonterminal(0);
		return std::move(n->value);
	});
	p->add_production("group_literal_char", { "DASH" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		(void) m;
		return std::unique_ptr<ParserAST>(new ParserASTRegex(std::unique_ptr<RegexAST>(new RegexASTLiteral('-'))));
	});
	p->add_production("group_literal_char", { "GROUP_ANY" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedTerminal* n = m->terminal(0);
		Long ch = utf8::codepoint_from_string(n->token->lexeme, 0, nullptr);
		assert(ch >= 0);
		return std::unique_ptr<ParserAST>(new ParserASTRegex(std::unique_ptr<RegexAST>(new RegexASTLiteral(ch))));
	});

	p->add_production("group_escaped_literal", { "ESCAPE", "group_escape_code" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedNonterminal* n = m->nonterminal(1);
		return std::move(n->value);
	});
	p->add_production("group_escape_code", { "group_escape_special" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedNonterminal* n = m->nonterminal(0);
		return std::move(n->value);
	});
	p->add_production("group_escape_code", { "escape_absolute" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedNonterminal* n = m->nonterminal(0);
		return std::move(n->value);
	});
	RegexEngine::add_literal(p.get(), "group_escape_special", "N", '\n');
	RegexEngine::add_literal(p.get(), "group_escape_special", "T", '\t');
	RegexEngine::add_literal(p.get(), "group_escape_special", "ESCAPE", '\\');
	//RegexEngine::add_literal(p.get(), "group_escape_special", "DASH", '-');
	RegexEngine::add_literal(p.get(), "group_escape_special", "LBRACKET", '[');
	RegexEngine::add_literal(p.get(), "group_escape_special", "RBRACKET", ']');

	p->add_production("escaped_literal", { "ESCAPE", "escape_code" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedNonterminal* n = m->nonterminal(1);
		return std::move(n->value);
	});
	p->add_production("escape_code", { "escape_special" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedNonterminal* n = m->nonterminal(0);
		return std::move(n->value);
	});
	RegexEngine::add_literal(p.get(), "escape_special", "N", '\n');
	RegexEngine::add_literal(p.get(), "escape_special", "T", '\t');
	RegexEngine::add_literal(p.get(), "escape_special", "ESCAPE", '\\');
	RegexEngine::add_literal(p.get(), "escape_special", "LPAREN", '(');
	RegexEngine::add_literal(p.get(), "escape_special", "RPAREN", ')');
	RegexEngine::add_literal(p.get(), "escape_special", "LBRACE", '{');
	RegexEngine::add_literal(p.get(), "escape_special", "RBRACE", '}');
	RegexEngine::add_literal(p.get(), "escape_special", "LBRACKET", '[');
	RegexEngine::add_literal(p.get(), "escape_special", "RBRACKET", ']');
	RegexEngine::add_literal(p.get(), "escape_special", "STAR", '*');
	RegexEngine::add_literal(p.get(), "escape_special", "PLUS", '+');
	RegexEngine::add_literal(p.get(), "escape_special", "QUESTION", '?');
	RegexEngine::add_literal(p.get(), "escape_special", "OR", '|');
	RegexEngine::add_literal(p.get(), "escape_special", "DOT", '.');
	p->add_production("escape_code", { "escape_absolute" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedNonterminal* n = m->nonterminal(0);
		return std::move(n->value);
	});
	p->add_production("escape_absolute", { "X", "hex_int_short" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::string str = dynamic_cast<ParserASTString*>(m->nonterminal(1)->value.get())->str;
		Long l = std::stol(str, nullptr, 16);
		assert(l >= 0);
		return std::unique_ptr<ParserAST>(new ParserASTRegex(std::unique_ptr<RegexAST>(new RegexASTLiteral((UInt) l))));
	});
	p->add_production("hex_int_short", { "hex_digit", "hex_digit" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::string str1 = dynamic_cast<ParserASTString*>(m->nonterminal(0)->value.get())->str;
		std::string str2 = dynamic_cast<ParserASTString*>(m->nonterminal(1)->value.get())->str;
		return std::unique_ptr<ParserAST>(new ParserASTString(str1 + str2));
	});
	p->add_production("escape_absolute", { "U", "hex_int_long" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::string str = dynamic_cast<ParserASTString*>(m->nonterminal(1)->value.get())->str;
		Long l = std::stol(str, nullptr, 16);
		assert(l >= 0);
		return std::unique_ptr<ParserAST>(new ParserASTRegex(std::unique_ptr<RegexAST>(new RegexASTLiteral((UInt) l))));
	});
	p->add_production("hex_int_long", {
		"hex_digit", "hex_digit", "hex_digit", "hex_digit",
		"hex_digit", "hex_digit", "hex_digit", "hex_digit",
	}, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::string str = "";
		for (Int i = 0; i < 8; i++) {
			str += dynamic_cast<ParserASTString*>(m->nonterminal(i)->value.get())->str;
		}
		std::string str1 = dynamic_cast<ParserASTString*>(m->nonterminal(0)->value.get())->str;
		std::string str2 = dynamic_cast<ParserASTString*>(m->nonterminal(1)->value.get())->str;
		return std::unique_ptr<ParserAST>(new ParserASTString(str));
	});
	/*
	p->add_production("hex_digit", { "DEC" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		return std::unique_ptr<ParserAST>(new ParserASTString(m->terminal(0)->token->lexeme));
	});
	*/
	p->add_production("hex_digit", { "HEX" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		return std::unique_ptr<ParserAST>(new ParserASTString(m->terminal(0)->token->lexeme));
	});

	p->add_production("mul_range", { "LBRACE", "dec_int", "COMMA", "dec_int", "RBRACE" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedNonterminal* n1 = m->nonterminal(1);
		MatchedNonterminal* n2 = m->nonterminal(3);
		ParserASTString* r1 = dynamic_cast<ParserASTString*>(n1->value.get());
		ParserASTString* r2 = dynamic_cast<ParserASTString*>(n2->value.get());
		Long l1 = std::stol(r1->str.c_str(), nullptr, 10);
		Long l2 = std::stol(r2->str.c_str(), nullptr, 10);
		return std::unique_ptr<ParserAST>(new ParserASTRange(l1, l2));
	});
	p->add_production("dec_int", { "DEC", "dec_int" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::string str1 = m->terminal(0)->token->lexeme;
		std::string str2 = dynamic_cast<ParserASTString*>(m->nonterminal(1)->value.get())->str;
		return std::unique_ptr<ParserAST>(new ParserASTString(str1 + str2));
	});
	p->add_production("dec_int", { "DEC" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		return std::unique_ptr<ParserAST>(new ParserASTString(m->terminal(0)->token->lexeme));
	});

	p->generate("regex");
	return p;
}

void RegexEngine::add_literal(Parser* p, std::string nonterminal, std::string token, UInt ch) {
	p->add_production(nonterminal, { token }, [ch](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		(void) m;
		return std::unique_ptr<ParserAST>(new ParserASTRegex(std::unique_ptr<RegexAST>(new RegexASTLiteral(ch))));
	});
}
