#ifndef SIYU_PARSER_H_INCLUDED
#define SIYU_PARSER_H_INCLUDED

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

/*
typedef DFAState<std::string, UInt> GrammarDFAState;
typedef NFAState<std::string, UInt> GrammarNFAState;
typedef DFA<std::string, UInt> GrammarDFA;
typedef NFA<std::string, UInt> GrammarNFA;
*/

struct Production;
class Match;

typedef std::function<void(Match*)> ProductionHandler;
typedef std::pair<Production*, UInt> Item;
typedef std::string Symbol;
typedef std::tuple<Symbol, Int, Int> ExtendedSymbol;

struct Production {
	std::string target;
	std::vector<std::string> symbols;
	ProductionHandler handler;
	bool nullable;
	bool empty;
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

	MatchedNonterminal(Production*);
};

class Parser {
	Lexer lexer;
	std::stack<std::unique_ptr<Token>> token_buffer;

	std::set<std::string> terminals;
	std::map<std::string, std::vector<Production*>> nonterminals;
	std::vector<std::unique_ptr<Production>> productions;
	std::string start;

	std::vector<std::unique_ptr<ItemSet>> states;
	std::map<std::set<Item>, ItemSet*> itemsets;

	std::map<std::string, std::set<std::string>> firsts;
	std::map<std::string, std::set<std::string>> follows;

	std::vector<std::unique_ptr<ExtendedProduction>> extended_grammar;
	std::map<ExtendedSymbol, std::vector<ExtendedProduction*>> extended_nonterminals;
	std::map<ExtendedSymbol, std::set<Symbol>> extended_firsts;
	std::map<ExtendedSymbol, std::set<Symbol>> extended_follows;

	std::vector<std::map<Symbol, Production*>> reductions;
	std::vector<std::map<Symbol, void*>> action_goto_table;
	std::stack<Int> parse_stack;
	std::stack<std::unique_ptr<Match>> parse_stack_matches;

	void push_token(std::unique_ptr<Token>);
	std::unique_ptr<Token> next_token(std::istream*);

	void generate();
	void generate(std::string);
	ItemSet* generate_itemset(std::set<Item>);
	void expand_symbol_into_itemset(ItemSet*, std::string, std::set<std::string>*);

	void generate_first_and_follow();
	void generate_first_set(std::string);
	void generate_follow_sets();

	void generate_extended_grammar();
	void generate_extended_first_and_follow();
	void generate_extended_first_set(ExtendedSymbol);
	void generate_extended_follow_sets();

	void generate_reductions();
	void generate_actions_and_gotos();

	// TODO: decide which convention better :P
	static bool is_item_done(Item);
	static bool symbol_is_token(std::string);
	static bool symbol_is_epsilon(std::string);
	bool symbol_is_nullable(std::string);

	static const std::string END;
	static const std::string EPSILON;
public:
	void set_start(std::string);
	void add_token(std::string, std::string);
	void add_production(std::string, std::vector<std::string>, ProductionHandler);
	void parse(std::istream*);

	static void debug(Parser*);
	static void debug_production(Production*, Int = -1);
	static void debug_item(Item);
	static void debug_extended_symbol(ExtendedSymbol);
	static void debug_extended_production(ExtendedProduction*);
	static void debug_set(std::set<std::string>);
	static void debug_match(Match*, Int);
};

#endif /* SIYU_PARSER_H_INCLUDED */
