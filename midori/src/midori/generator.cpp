#include "generator.h"

#include "regex_engine.h"
#include <vector>

typedef std::vector<std::string> StringList;
typedef std::vector<StringList> StringListList;
typedef ParserValue<StringList> ParserValueStringList;
typedef ParserValue<StringListList> ParserValueStringListList;

std::unique_ptr<Parser> ParserGenerator::from_file(std::istream* is) {
	RegexEngine re;
	std::unique_ptr<Parser> ret(new Parser);
	std::unique_ptr<Parser> p(new Parser);
	std::string start;

	p->add_token("WHITESPACE", re.parse("[ \\t]"));
	p->add_token("NL", re.parse("\\n"));
	p->add_token("PERCENT", re.parse("%"));
	p->add_token("START", re.parse("start"));
	p->add_token("SKIP", re.parse("skip"));
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

	p->add_production("line", { "line_start" }, nullptr);
	p->add_production("line", { "line_skip" }, nullptr);
	p->add_production("line", { "line_token" }, nullptr);
	p->add_production("line", { "line_production" }, nullptr);
	p->add_production("line", {}, nullptr);

	p->add_production("line_start", { "PERCENT", "START", "COLON", "NONTERMINAL" }, [ &start ](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		start = m->terminal(3)->token->lexeme;
		return nullptr;
	});

	p->add_production("line_skip", { "PERCENT", "SKIP", "COLON", "TOKEN" }, [ &ret ](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::string skip = m->terminal(3)->token->lexeme;
		ret->add_skip(skip);
		return nullptr;
	});

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

	p->add_production("nl_optional", { "NL" }, nullptr);
	p->add_production("nl_optional", {}, nullptr);

	p->add_production("line_production", { "NONTERMINAL", "nl_optional", "COLON", "production_list", "nl_optional", "SEMICOLON" }, [ &ret ](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::string target = m->terminal(0)->token->lexeme;
		StringListList& l = m->nonterminal(3)->value->get<StringListList>();
		for (std::vector<std::string> const& p : l) {
			ret->add_production(target, p, nullptr);
		}
		return nullptr;
	});

	p->add_production("production_list", { "production_list", "nl_optional", "BAR", "production" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::unique_ptr<ParserAST> n1 = std::move(m->nonterminal(0)->value);
		std::unique_ptr<ParserAST> n2 = std::move(m->nonterminal(3)->value);
		StringListList& l = n1->get<StringListList>();
		l.push_back(n2->get<StringList>());
		return n1;
	});
	p->add_production("production_list", { "nl_optional", "production" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::unique_ptr<ParserAST> n = std::move(m->nonterminal(1)->value);
		StringList& l = n->get<StringList>();
		return std::unique_ptr<ParserAST>(new ParserValueStringListList({ l }));
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
		return std::unique_ptr<ParserAST>(new ParserValueStringList({}));
	});

	p->add_production("symbol_list", { "symbol_list", "symbol" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::unique_ptr<ParserAST> n = std::move(m->nonterminal(0)->value);
		StringList& l = n->get<StringList>();
		std::string str = m->nonterminal(1)->value->get<std::string>();
		l.push_back(str);
		return n;
	});
	p->add_production("symbol_list", { "symbol" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		std::string str = m->nonterminal(0)->value->get<std::string>();
		return std::unique_ptr<ParserAST>(new ParserValueStringList({ str }));
	});

	p->add_production("symbol", { "TOKEN" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		return std::unique_ptr<ParserAST>(new ParserValue<std::string>(m->terminal(0)->token->lexeme));
	});
	p->add_production("symbol", { "NONTERMINAL" }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		return std::unique_ptr<ParserAST>(new ParserValue<std::string>(m->terminal(0)->token->lexeme));
	});

	p->generate(Parser::Type::LALR1, "grammar");
	FileInputStream fis(is);
	std::unique_ptr<MatchedNonterminal> m = p->parse(&fis);
	if (start.length() == 0) {
		std::cout << "No start rule" << std::endl;
		return nullptr;
	}
	ret->generate(Parser::Type::LALR1, start);
	return ret;
}
