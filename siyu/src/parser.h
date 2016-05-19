#ifndef SIYU_PARSER_H_INCLUDED
#define SIYU_PARSER_H_INCLUDED

#include <map>
#include <vector>
#include <set>
#include <stack>
#include <functional>
#include <utility>
#include "global.h"
#include "lexer.h"

// TODO: epsilon
// TODO: named matches

typedef DFAState<std::string, UInt> GrammarDFAState;
typedef NFAState<std::string, UInt> GrammarNFAState;
typedef DFA<std::string, UInt> GrammarDFA;
typedef NFA<std::string, UInt> GrammarNFA;

typedef std::function<void()> ProductionHandler;

struct Production {
	std::string target;
	std::vector<std::string> symbols;
	ProductionHandler handler;
};

typedef std::pair<Production*, UInt> Item;

struct ItemSet {
	std::set<Item> head;
	std::set<Item> additionals;
	std::map<std::string, ItemSet*> next;
	Int index;
};

class Parser {
	Lexer lexer;
	std::stack<Token*> token_buffer;

	std::set<std::string> terminals;
	std::map<std::string, std::vector<Production*>> nonterminals;
	std::vector<std::unique_ptr<Production>> productions;
	std::string start;

	std::vector<std::unique_ptr<ItemSet>> states;
	std::map<std::set<Item>, ItemSet*> itemsets;

	void push_token(Token*);
	Token* next_token(std::istream*);

	void generate();
	void generate(std::string);
	ItemSet* generate_itemset(std::set<Item>);
	void expand_symbol_into_itemset(ItemSet*, std::string, std::set<std::string>*);

	// TODO: decide which convention better :P
	static bool is_item_done(Item);
	static bool symbol_is_token(std::string);

	static void debug(Parser*);
	static void debug_production(Production*, Int = -1);
	static void debug_item(Item);
public:
	void set_start(std::string);
	void add_token(std::string, std::string);
	void add_production(std::string, std::vector<std::string>, ProductionHandler);
	void parse(std::istream*);
};

#endif /* SIYU_PARSER_H_INCLUDED */
