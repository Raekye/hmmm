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

struct Production;
class MatchedNonterminal;
class ParserAST;

typedef std::function<std::unique_ptr<ParserAST>(MatchedNonterminal*)> ProductionHandler;
typedef std::pair<Production*, Int> Item;
typedef std::string Symbol;
typedef std::tuple<Symbol, Int, Int> ExtendedSymbol;

class ParserAST {
public:
	virtual ~ParserAST() = 0;
};

struct Production {
	std::string target;
	std::vector<std::string> symbols;
	ProductionHandler handler;
};

struct ExtendedProduction {
	ExtendedSymbol target;
	std::vector<ExtendedSymbol> symbols;
	Production* orig;
};

struct ItemSet {
	std::set<Item> head;
	std::set<Item> additionals;
	std::map<std::string, ItemSet*> next;
	Int index;
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

	MatchedNonterminal(Production*);
	inline MatchedTerminal* terminal(Int i) {
		return dynamic_cast<MatchedTerminal*>(this->terms[i].get());
	}
	inline MatchedNonterminal* nonterminal(Int i) {
		return dynamic_cast<MatchedNonterminal*>(this->terms[i].get());
	}
};

class Parser {
	static const std::string END;
	static const std::string EPSILON;

	static bool symbol_is_token(std::string);
	static bool symbol_is_epsilon(std::string);
	static bool production_is_epsilon(Production*);
	static bool item_is_done(Item);

	Lexer lexer;
	std::stack<std::unique_ptr<Token>> token_buffer;

	std::set<std::string> terminals;
	std::map<std::string, std::vector<Production*>> nonterminals;
	std::vector<std::unique_ptr<Production>> productions;
	std::string start;

	std::vector<std::unique_ptr<ItemSet>> states;
	std::map<std::set<Item>, ItemSet*> itemsets;

	std::vector<std::unique_ptr<ExtendedProduction>> extended_grammar;
	std::map<ExtendedSymbol, std::vector<ExtendedProduction*>> extended_nonterminals;
	std::map<ExtendedSymbol, std::set<Symbol>> extended_firsts;
	std::map<ExtendedSymbol, std::set<Symbol>> extended_follows;

	std::vector<std::map<Symbol, Production*>> reductions;
	std::stack<Int> parse_stack;
	std::stack<std::unique_ptr<Match>> parse_stack_matches;

	void reset();

	void push_token(std::unique_ptr<Token>);
	std::unique_ptr<Token> next_token(std::istream*);

	ItemSet* generate_itemset(std::set<Item>);
	void expand_symbol_into_itemset(ItemSet*, std::string, std::set<std::string>*);

	void generate_extended_grammar();
	void generate_extended_first_sets();
	void generate_extended_follow_sets();

	void generate_reductions();

public:
	void set_start(std::string);
	void add_token(std::string, std::string, std::unique_ptr<RegexAST>);
	void add_production(std::string, std::vector<std::string>, ProductionHandler);
	void generate(std::string);
	std::unique_ptr<Match> parse(std::istream*);

	static void debug(Parser*);
	static void debug_production(Production*, Int = -1);
	static void debug_item(Item);
	static void debug_extended_symbol(ExtendedSymbol);
	static void debug_extended_production(ExtendedProduction*);
	static void debug_set(std::set<std::string>);
	static void debug_match(Match*, Int);
};

#endif /* MIDORI_PARSER_H_INCLUDED */
