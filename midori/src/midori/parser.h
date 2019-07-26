#ifndef MIDORI_PARSER_H_INCLUDED
#define MIDORI_PARSER_H_INCLUDED

#include <map>
#include <vector>
#include <set>
#include <stack>
#include <functional>
#include <utility>
#include <tuple>
#include "global.h"
#include "lexer.h"

// TODO: named matches
// TODO: use symbol typedef
// TODO: asserts
// TODO: unique ptr
// TODO: for loops const and &

struct Production;
class MatchedNonterminal;
class ParserAST;
class Parser;

typedef std::function<std::unique_ptr<ParserAST>(MatchedNonterminal*)> ProductionHandler;
typedef std::function<std::unique_ptr<MatchedNonterminal>(std::unique_ptr<MatchedNonterminal>)> RewriteHandler;
typedef std::pair<Production*, Int> Item;

class ParserAST {
public:
	virtual ~ParserAST() = 0;
};

class ParserASTString : public ParserAST {
public:
	std::string str;
	ParserASTString(std::string);
};

struct Production {
	std::string target;
	std::vector<std::string> symbols;
	ProductionHandler handler;
	RewriteHandler rewrite;
};

struct ItemSet {
	Int index;
	std::set<Item> kernel;
	std::set<Item> closure;
	std::map<std::string, ItemSet*> next;
	std::map<std::string, Production*> reductions;
};

class Match {
public:
	virtual ~Match() = 0;
};
class MatchedTerminal : public Match {
public:
	std::unique_ptr<Token> token;

	MatchedTerminal(std::unique_ptr<Token>);
};

class MatchedNonterminal : public Match {
public:
	Production* production;
	std::vector<std::unique_ptr<Match>> terms;
	std::unique_ptr<ParserAST> value;

	MatchedNonterminal();
	MatchedNonterminal(Production*);
	MatchedTerminal* terminal(Int i) {
		return dynamic_cast<MatchedTerminal*>(this->terms.at(i).get());
	}
	MatchedNonterminal* nonterminal(Int i) {
		return dynamic_cast<MatchedNonterminal*>(this->terms.at(i).get());
	}

private:
	MatchedNonterminal(Production*, size_t);
};

class Parser {
public:
	void add_token(std::string, std::unique_ptr<RegexAST>);
	void add_skip(std::string);
	void add_production(std::string, std::vector<std::string>, ProductionHandler);
	void add_production(std::string, std::vector<std::string>, ProductionHandler, RewriteHandler);
	void generate(std::string);
	std::unique_ptr<MatchedNonterminal> parse(IInputStream*);
	void reset();

	void debug();

private:
	static std::string const ROOT;

	static bool production_is_epsilon(Production*);
	static bool item_is_done(Item);

	Lexer lexer;

	std::set<std::string> terminals;
	std::map<std::string, std::vector<Production*>> nonterminals;
	std::vector<std::unique_ptr<Production>> productions;

	std::set<std::string> nullable;
	std::map<std::string, std::set<std::string>> firsts;
	std::map<std::string, std::set<std::string>> follows;

	std::vector<std::unique_ptr<ItemSet>> states;
	std::map<std::set<Item>, ItemSet*> itemsets;

	std::stack<std::unique_ptr<Match>> symbol_buffer;
	std::stack<Int> parse_stack;
	std::stack<std::unique_ptr<Match>> parse_stack_matches;

	ItemSet* current_state() {
		return this->states[this->parse_stack.top()].get();
	}
	bool symbol_is_token(std::string s) {
		return this->terminals.find(s) != this->terminals.end();
	}

	void push_symbol(std::unique_ptr<Match>);
	void pull_symbols(std::unique_ptr<Match>);
	std::unique_ptr<Match> next_symbol(IInputStream*);

	bool parse_advance(std::unique_ptr<Match>, bool*);
	std::unique_ptr<Match> parse_symbol(std::string, std::unique_ptr<Match>, bool*);

	void generate_first_sets();
	void generate_follow_sets();

	ItemSet* generate_itemset(std::set<Item>);
	void expand_symbol_into_itemset(ItemSet*, std::string, std::set<std::string>*);
	void generate_itemsets();
	void generate_closure(std::set<Item>*, std::set<Item>*);

	static void debug_production(Production*, Int = -1);
	static void debug_item(Item);
	static void debug_set(std::set<std::string>);
	static void debug_match(Match*, Int);
};

#endif /* MIDORI_PARSER_H_INCLUDED */
