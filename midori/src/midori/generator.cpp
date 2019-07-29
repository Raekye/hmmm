#include "generator.h"

#include "regex_engine.h"
#include <vector>

class ParserASTStringList : public ParserAST {
public:
	std::vector<std::string> list;
	ParserASTStringList(std::vector<std::string> list) : list(list) {
		return;
	}
};

class ParserASTStringListList : public ParserAST {
public:
	std::vector<std::vector<std::string>> list;
	ParserASTStringListList(std::vector<std::vector<std::string>> list) : list(list) {
		return;
	}
};

std::unique_ptr<Parser> ParserGenerator::from_file(std::istream* is) {
	RegexEngine re;
	std::unique_ptr<Parser> ret(new Parser);
	std::unique_ptr<Parser> p(new Parser);
	p->add_token("WHITESPACE", re.parse("[ \\t]"));
	p->add_token("NL", re.parse("\\n"));
	p->add_token("TOKEN", re.parse("[A-Z][a-zA-Z0-9_]*"));
	p->add_token("REGEX", re.parse("/[^\\n]+"));
	p->add_token("NONTERMINAL", re.parse("[a-z][a-zA-Z0-9_]*"));
	p->add_token("COLON", re.parse(":"));
	p->add_token("SEMICOLON", re.parse(";"));
	p->add_token("BAR", re.parse("\\|"));
	p->add_token("EPSILON", re.parse("$epsilon"));
	p->add_skip("WHITESPACE");

	p->add_production("grammar", { "contents" }, nullptr);

	p->add_production("contents", { "line", "NL", "contents" }, nullptr);
	p->add_production("contents", { "line" }, nullptr);

	p->add_production("line", { "line_token" }, nullptr);
	p->add_production("line", { "line_production" }, nullptr);
	p->add_production("line", {}, nullptr);

	p->add_production("line_token", { "TOKEN", "COLON", "REGEX" }, [ &ret, &re ](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::string t = m->terminal(0)->token->lexeme;
		std::string r = m->terminal(2)->token->lexeme.substr(1);
		std::unique_ptr<RegexAST> regex = re.parse(r);
		if (regex == nullptr) {
			// TODO
			std::cout << "Bad regex " << r << std::endl;
		}
		ret->add_token(t, std::move(regex));
		return nullptr;
	});

	p->add_production("line_production", { "NONTERMINAL", "COLON", "production_list", "SEMICOLON" }, [ &ret ](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::string target = m->terminal(0)->token->lexeme;
		std::unique_ptr<ParserAST> n = std::move(m->nonterminal(2)->value);
		ParserASTStringListList* l = dynamic_cast<ParserASTStringListList*>(n.get());
		for (std::vector<std::string> const& p : l->list) {
			ret->add_production(target, p, nullptr);
		}
		return nullptr;
	});

	p->add_production("nl_optional", { "NL" }, nullptr);
	p->add_production("nl_optional", {}, nullptr);

	p->add_production("production_list", { "production_list", "nl_optional", "BAR", "production" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::unique_ptr<ParserAST> n1 = std::move(m->nonterminal(0)->value);
		std::unique_ptr<ParserAST> n2 = std::move(m->nonterminal(3)->value);
		ParserASTStringListList* l = dynamic_cast<ParserASTStringListList*>(n1.get());
		l->list.push_back(dynamic_cast<ParserASTStringList*>(n2.get())->list);
		return n1;
	});
	p->add_production("production_list", { "nl_optional", "production" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::unique_ptr<ParserAST> n = std::move(m->nonterminal(1)->value);
		ParserASTStringList* l = dynamic_cast<ParserASTStringList*>(n.get());
		return std::unique_ptr<ParserAST>(new ParserASTStringListList({ l->list }));
	});
	/*
	 * TODO: why doesn't the following work?
		p->add_production("production_list", { "nl_optional", "production", "nl_optional", "BAR", "production_list" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
			return nullptr;
		});
		p->add_production("production_list", { "production" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
			return nullptr;
		});
	*/

	p->add_production("production", { "symbol_list" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		return std::move(m->nonterminal(0)->value);
	});
	p->add_production("production", { "EPSILON" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		(void) m;
		return std::unique_ptr<ParserAST>(new ParserASTStringList({}));
	});

	p->add_production("symbol_list", { "symbol_list", "symbol" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::unique_ptr<ParserAST> n = std::move(m->nonterminal(0)->value);
		ParserASTStringList* l = dynamic_cast<ParserASTStringList*>(n.get());
		ParserASTString* s = dynamic_cast<ParserASTString*>(m->nonterminal(1)->value.get());
		l->list.push_back(s->str);
		return n;
	});
	p->add_production("symbol_list", { "symbol" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		ParserASTString* s = dynamic_cast<ParserASTString*>(m->nonterminal(0)->value.get());
		return std::unique_ptr<ParserAST>(new ParserASTStringList({ s->str }));
	});

	p->add_production("symbol", { "TOKEN" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		return std::unique_ptr<ParserAST>(new ParserASTString(m->terminal(0)->token->lexeme));
	});
	p->add_production("symbol", { "NONTERMINAL" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		return std::unique_ptr<ParserAST>(new ParserASTString(m->terminal(0)->token->lexeme));
	});

	p->generate(Parser::Type::LALR1, "grammar");
	FileInputStream fis(is);
	std::unique_ptr<MatchedNonterminal> m = p->parse(&fis);
	ret->generate(Parser::Type::LALR1, "root");
	return ret;
}
