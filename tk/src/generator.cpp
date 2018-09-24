#include "generator.h"

#include "helper.h"

ParserRegexAST::ParserRegexAST(std::unique_ptr<RegexAST> r) : regex(std::move(r)) {
	return;
}
ParserStringAST::ParserStringAST(std::string s) : str(s) {
	return;
}
ParserRangeAST::ParserRangeAST(Long a, Long b) : min(a), max(b) {
	return;
}

std::unique_ptr<Parser> RegexParserGenerator::make() {
	std::unique_ptr<Parser> p(new Parser);
	p->add_token("STAR", "", std::unique_ptr<RegexAST>(new RegexASTLiteral('*')));
	p->add_token("PLUS", "", std::unique_ptr<RegexAST>(new RegexASTLiteral('+')));
	p->add_token("QUESTION", "", std::unique_ptr<RegexAST>(new RegexASTLiteral('?')));
	p->add_token("OR", "", std::unique_ptr<RegexAST>(new RegexASTLiteral('|')));
	p->add_token("ESCAPE", "", std::unique_ptr<RegexAST>(new RegexASTLiteral('\\')));
	p->add_token("DOT", "", std::unique_ptr<RegexAST>(new RegexASTLiteral('.')));

	p->add_token("LPAREN", "", std::unique_ptr<RegexAST>(new RegexASTLiteral('(')));
	p->add_token("RPAREN", "", std::unique_ptr<RegexAST>(new RegexASTLiteral(')')));
	p->add_token("LBRACE", "", std::unique_ptr<RegexAST>(new RegexASTLiteral('{')));
	p->add_token("RBRACE", "", std::unique_ptr<RegexAST>(new RegexASTLiteral('}')));
	p->add_token("LBRACKET", "", std::unique_ptr<RegexAST>(new RegexASTLiteral('[')));
	p->add_token("RBRACKET", "", std::unique_ptr<RegexAST>(new RegexASTLiteral(']')));

	p->add_token("DASH", "", std::unique_ptr<RegexAST>(new RegexASTLiteral('-')));
	p->add_token("COMMA", "", std::unique_ptr<RegexAST>(new RegexASTLiteral(',')));

	p->add_token("X", "", std::unique_ptr<RegexAST>(new RegexASTLiteral('x')));
	p->add_token("T", "", std::unique_ptr<RegexAST>(new RegexASTLiteral('t')));
	p->add_token("N", "", std::unique_ptr<RegexAST>(new RegexASTLiteral('n')));

	p->add_token("ANY", "", std::unique_ptr<RegexAST>(new RegexASTWildcard));

	p->set_start("regex");
	p->add_production("regex", { "lr_or" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedNonterminal* n = m->nonterminal(0);
		return std::move(n->value);
	});
	p->add_production("lr_or", { "lr_add", "OR", "lr_or" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedNonterminal* n1 = m->nonterminal(0);
		MatchedNonterminal* n2 = m->nonterminal(2);
		ParserRegexAST* r1 = dynamic_cast<ParserRegexAST*>(n1->value.get());
		ParserRegexAST* r2 = dynamic_cast<ParserRegexAST*>(n2->value.get());
		return std::unique_ptr<ParserAST>(new ParserRegexAST(std::unique_ptr<RegexAST>(new RegexASTOr(std::move(r1->regex), std::move(r2->regex)))));
	});
	p->add_production("lr_or", { "lr_add" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedNonterminal* n = m->nonterminal(0);
		return std::move(n->value);
	});
	p->add_production("lr_add", { "lr_mul", "lr_add" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedNonterminal* n1 = m->nonterminal(0);
		MatchedNonterminal* n2 = m->nonterminal(1);
		ParserRegexAST* r1 = dynamic_cast<ParserRegexAST*>(n1->value.get());
		ParserRegexAST* r2 = dynamic_cast<ParserRegexAST*>(n2->value.get());
		std::vector<std::unique_ptr<RegexAST>> vec;
		vec.push_back(std::move(r1->regex));
		vec.push_back(std::move(r2->regex));
		std::unique_ptr<RegexAST> ret(new RegexASTChain(std::move(vec)));
		// The following does not work
		/*
		std::unique_ptr<RegexAST> ret(new RegexASTChain({
			std::move(r1->regex),
			//std::move(r2->regex),
		}));
		*/
		return std::unique_ptr<ParserAST>(new ParserRegexAST(std::move(ret)));
	});
	p->add_production("lr_add", { "lr_mul" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedNonterminal* n = m->nonterminal(0);
		return std::move(n->value);
	});

	p->add_production("lr_mul", { "not_lr", "STAR" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedNonterminal* n1 = m->nonterminal(0);
		ParserRegexAST* r = dynamic_cast<ParserRegexAST*>(n1->value.get());
		std::unique_ptr<RegexAST> ret(new RegexASTMultiplication(std::move(r->regex), 0, 0));
		return std::unique_ptr<ParserAST>(new ParserRegexAST(std::move(ret)));
	});
	p->add_production("lr_mul", { "not_lr", "PLUS" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedNonterminal* n1 = m->nonterminal(0);
		ParserRegexAST* r = dynamic_cast<ParserRegexAST*>(n1->value.get());
		std::unique_ptr<RegexAST> ret(new RegexASTMultiplication(std::move(r->regex), 1, 0));
		return std::unique_ptr<ParserAST>(new ParserRegexAST(std::move(ret)));
	});
	p->add_production("lr_mul", { "not_lr", "QUESTION" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedNonterminal* n1 = m->nonterminal(0);
		ParserRegexAST* r = dynamic_cast<ParserRegexAST*>(n1->value.get());
		std::unique_ptr<RegexAST> ret(new RegexASTMultiplication(std::move(r->regex), 0, 1));
		return std::unique_ptr<ParserAST>(new ParserRegexAST(std::move(ret)));
	});
	p->add_production("lr_mul", { "not_lr", "mul_range" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedNonterminal* n1 = m->nonterminal(0);
		MatchedNonterminal* n2 = m->nonterminal(1);
		ParserRegexAST* r = dynamic_cast<ParserRegexAST*>(n1->value.get());
		ParserRangeAST* range = dynamic_cast<ParserRangeAST*>(n2->value.get());
		std::unique_ptr<RegexAST> ret(new RegexASTMultiplication(std::move(r->regex), (UInt) range->min, (UInt) range->max));
		return std::unique_ptr<ParserAST>(new ParserRegexAST(std::move(ret)));
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

	p->add_production("parentheses", { "LPAREN", "lr_or", "RPAREN" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedNonterminal* n = m->nonterminal(1);
		return std::move(n->value);
	});

	p->add_production("literal", { "escaped_literal" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedNonterminal* n = m->nonterminal(0);
		return std::move(n->value);
	});
	p->add_production("literal", { "ANY" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedTerminal* n = m->terminal(0);
		return std::unique_ptr<ParserAST>(new ParserRegexAST(std::unique_ptr<RegexAST>(new RegexASTLiteral(n->token->lexeme.at(0)))));
	});

	p->add_production("group", { "LBRACKET", "group_contents", "RBRACKET" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedNonterminal* n = m->nonterminal(1);
		return std::move(n->value);
	});
	p->add_production("group_contents", { "group_element", "group_contents" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedNonterminal* n1 = m->nonterminal(0);
		MatchedNonterminal* n2 = m->nonterminal(1);
		ParserRegexAST* r1 = dynamic_cast<ParserRegexAST*>(n1->value.get());
		ParserRegexAST* r2 = dynamic_cast<ParserRegexAST*>(n2->value.get());
		return std::unique_ptr<ParserAST>(new ParserRegexAST(std::unique_ptr<RegexAST>(new RegexASTOr(std::move(r1->regex), std::move(r2->regex)))));
	});
	p->add_production("group_contents", { "group_element" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedNonterminal* n = m->nonterminal(0);
		return std::move(n->value);
	});
	p->add_production("group_element", { "group_range" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedNonterminal* n = m->nonterminal(0);
		return std::move(n->value);
	});
	p->add_production("group_element", { "literal" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		MatchedNonterminal* n = m->nonterminal(0);
		return std::move(n->value);
	});
	p->add_production("group_range", { "literal", "DASH", "literal" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		(void) m;
		return nullptr;
	});

	p->add_production("escaped_literal", { "ESCAPE", "escape_code" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		(void) m;
		return nullptr;
	});
	p->add_production("escape_code", { "ANY", "escape_code" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::string str1 = m->terminal(0)->token->lexeme;
		std::string str2 = dynamic_cast<ParserStringAST*>(m->nonterminal(1)->value.get())->str;
		return std::unique_ptr<ParserAST>(new ParserStringAST(str1 + str2));
	});
	p->add_production("escape_code", { "ANY" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		return std::unique_ptr<ParserAST>(new ParserStringAST(m->terminal(0)->token->lexeme));
	});

	p->add_production("mul_range", { "LBRACE", "dec_int", "COMMA", "dec_int", "RBRACE" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		mdk::printf("[debug] making mul range\n");
		MatchedNonterminal* n1 = m->nonterminal(1);
		MatchedNonterminal* n2 = m->nonterminal(3);
		ParserStringAST* r1 = dynamic_cast<ParserStringAST*>(n1->value.get());
		ParserStringAST* r2 = dynamic_cast<ParserStringAST*>(n2->value.get());
		Long l1 = std::stol(r1->str, nullptr, 10);
		Long l2 = std::stol(r2->str, nullptr, 10);
		mdk::printf("[debug] made mul range %ld to %ld\n", l1, l2);
		return std::unique_ptr<ParserAST>(new ParserRangeAST(l1, l2));
	});
	p->add_production("dec_int", { "ANY", "dec_int" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::string str1 = m->terminal(0)->token->lexeme;
		std::string str2 = dynamic_cast<ParserStringAST*>(m->nonterminal(1)->value.get())->str;
		return std::unique_ptr<ParserAST>(new ParserStringAST(str1 + str2));
	});
	p->add_production("dec_int", { "ANY" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		return std::unique_ptr<ParserAST>(new ParserStringAST(m->terminal(0)->token->lexeme));
	});

	return p;
}
