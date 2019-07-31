#ifndef MIDORI_PARSER_H_INCLUDED
#define MIDORI_PARSER_H_INCLUDED

#include <map>
#include <vector>
#include <set>
#include <stack>
#include <list>
#include <tuple>
#include <functional>
#include "global.h"
#include "lexer.h"

class ParserAST;
class MatchedNonterminal;
struct Production;

typedef std::function<std::unique_ptr<ParserAST>(MatchedNonterminal*)> ProductionHandler;
typedef std::function<std::unique_ptr<MatchedNonterminal>(std::unique_ptr<MatchedNonterminal>)> RewriteHandler;

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
	Int index;
	std::string target;
	std::vector<std::string> symbols;
	ProductionHandler handler;
	RewriteHandler rewrite;

	bool is_epsilon() const {
		return this->symbols.size() == 0;
	}
};

struct Item {
	Production* production;
	Int dot;
	// for LR0 items, `terminal` is unused
	std::string terminal;

	Item(Production* p, Int d, std::string t) : production(p), dot(d), terminal(t) {
		return;
	}

	bool is_done() const {
		return this->dot == this->production->symbols.size();
	}

	std::string next_symbol() const {
		return this->production->symbols.at(this->dot);
	}

	friend bool operator<(Item const& lhs, Item const& rhs) {
		return std::tie(lhs.production->index, lhs.dot, lhs.terminal) < std::tie(rhs.production->index, rhs.dot, rhs.terminal);
	}
};

struct ItemSet {
	Int index;
	bool accept;
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

struct GrammarConflict {
	enum Type {
		ShiftReduce,
		ReduceReduce,
	};

	Type type;
	ItemSet* state;
	std::string symbol;
	std::vector<Production*> productions;
};

class Parser {
public:
	enum Type {
		LALR1,
		LR1,
	};

	void add_token(std::string, std::unique_ptr<RegexAST>);
	void add_skip(std::string);
	void add_production(std::string, std::vector<std::string>, ProductionHandler);
	void add_production(std::string, std::vector<std::string>, ProductionHandler, RewriteHandler);
	void generate(Type, std::string);
	std::unique_ptr<MatchedNonterminal> parse(IInputStream*);
	std::vector<GrammarConflict> conflicts();
	void reset();

	void debug();

private:
	typedef std::map<Item, std::unique_ptr<std::set<std::string>>> LookaheadMap;
	typedef std::map<Item, std::vector<std::set<std::string>*>> PropagateLookaheadMap;

	static std::string const ROOT;
	static std::string const TOKEN_MIDORI;

	Lexer lexer;

	std::set<std::string> terminals;
	std::map<std::string, std::vector<Production*>> nonterminals;
	std::vector<std::unique_ptr<Production>> productions;

	std::set<std::string> nullable;
	std::map<std::string, std::set<std::string>> firsts;

	std::vector<std::unique_ptr<ItemSet>> lr0_states;
	std::map<std::set<Item>, ItemSet*> lr0_itemsets;

	std::vector<std::unique_ptr<ItemSet>> lr1_states;
	std::map<std::set<Item>, ItemSet*> lr1_itemsets;

	std::vector<LookaheadMap> lookaheads;
	std::vector<PropagateLookaheadMap> lookahead_propagates;

	std::vector<GrammarConflict> _conflicts;

	std::stack<std::unique_ptr<Match>> symbol_buffer;
	std::stack<ItemSet*> parse_stack_states;
	std::stack<std::unique_ptr<Match>> parse_stack_matches;

	bool symbol_is_token(std::string s) {
		return this->terminals.find(s) != this->terminals.end();
	}

	void generate_first_sets();
	// for LALR(1), first generate the LR(0) itemsets,
	// then calculate the lookaheads for each kernel item,
	// attach the lookaheads and generate the LR(1) closure
	// see the Dragon book pages 270 - 273
	void generate_lr0_closure(ItemSet*);
	void generate_lr1_closure(ItemSet*);
	void generate_itemsets(Type);

	ItemSet* register_state(Type, std::unique_ptr<ItemSet>, std::list<ItemSet*>*);

	void discover_lookaheads();
	void propagate_lookaheads();
	void generate_lalr_itemsets();

	void generate_reduction(ItemSet*, Item);

	void push_symbol(std::unique_ptr<Match>);
	void pull_symbols(std::unique_ptr<Match>);
	std::unique_ptr<Match> next_symbol(IInputStream*);

	bool parse_advance(std::unique_ptr<Match>, bool*);
	std::unique_ptr<Match> parse_symbol(std::string, std::unique_ptr<Match>, bool*);

	static void debug_production(Production*, Int = -1, std::string = "");
	static void debug_item(Item);
	static void debug_set(std::set<std::string>);
	static void debug_match(Match*, Int);
};

#endif /* MIDORI_PARSER_H_INCLUDED */
