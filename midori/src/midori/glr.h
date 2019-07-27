#ifndef MIDORI_GLR_H_INCLUDED
#define MIDORI_GLR_H_INCLUDED

#include <map>
#include <vector>
#include <set>
#include <list>
#include <functional>
#include <utility>
#include "global.h"
#include "lexer.h"

namespace GLR {

	class MatchedNonterminal;
	class ParserAST;

	typedef std::function<std::unique_ptr<ParserAST>(MatchedNonterminal*)> ProductionHandler;
	typedef std::function<std::unique_ptr<MatchedNonterminal>(std::unique_ptr<MatchedNonterminal>)> RewriteHandler;

	class ParserAST {
	public:
		virtual ~ParserAST() = 0;
	};

	struct Production {
		Int id;
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

		Item(Production* p, Int d) : production(p), dot(d) {
			return;
		}

		bool is_done() const {
			return dot == this->production->symbols.size();
		}

		bool operator<(Item const& other) const {
			Int a = this->production->id;
			Int b = other.production->id;
			if (a < b) {
				return true;
			} else if (a > b) {
				return false;
			}
			return this->dot < other.dot;
		}
	};

	struct ItemSet {
		Int id;
		std::set<Item> kernel;
		std::set<Item> closure;
		std::map<std::string, ItemSet*> next;
		std::map<std::string, std::vector<Production*>> reductions;
	};

	class Match {
	public:
		virtual ~Match() = 0;
	};

	class MatchedTerminal : public Match {
	public:
		std::string tag;
		std::shared_ptr<Token> token;

		MatchedTerminal(std::string, std::shared_ptr<Token>);
	};

	class MatchedNonterminal : public Match {
	public:
		Production* production;
		std::vector<std::shared_ptr<Match>> terms;
		std::unique_ptr<ParserAST> value;

		MatchedNonterminal(Production*);
		MatchedTerminal* terminal(Int i) {
			return dynamic_cast<MatchedTerminal*>(this->terms.at(i).get());
		}
		MatchedNonterminal* nonterminal(Int i) {
			return dynamic_cast<MatchedNonterminal*>(this->terms.at(i).get());
		}
	};

	class MatchedForest : public Match {
	public:
		std::vector<std::unique_ptr<MatchedNonterminal>> matches;
	};

	class StackNode;
	typedef std::list<std::unique_ptr<StackNode>> StackNodeList;
	typedef StackNodeList::iterator StackNodeListNode;

	class StackNode {
	public:
		Int references;
		ItemSet* state;
		std::shared_ptr<Match> value;
		std::vector<StackNode*> left_siblings;
		std::vector<StackNode*> right_siblings;
		Int deterministic_depth;
		StackNodeListNode source;
	};

	class StackPath {
	public:
		StackNode* root;
		std::vector<StackNode*> nodes;
		Production* production;
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

		Lexer lexer;

		std::set<std::string> terminals;
		std::map<std::string, std::vector<Production*>> nonterminals;
		std::vector<std::unique_ptr<Production>> productions;

		std::set<std::string> nullable;
		std::map<std::string, std::set<std::string>> firsts;
		std::map<std::string, std::set<std::string>> follows;

		std::vector<std::unique_ptr<ItemSet>> states;
		std::map<std::set<Item>, ItemSet*> itemsets;

		StackNodeList stack_elements;
		std::vector<StackNode*> stack_top[2];
		UInt stack_top_index;
		std::vector<StackNode*> stack_top_map;

		std::vector<StackNode*>* stack_top_current() {
			return this->stack_top + this->stack_top_index;
		}
		std::vector<StackNode*>* stack_top_next() {
			return this->stack_top + (this->stack_top_index & 1);
		}
		StackNode* push_state(ItemSet* state) {
			StackNode** map = this->stack_top_map.data();
			if (map[state->id] != nullptr) {
				return map[state->id];
			}
			std::unique_ptr<StackNode> n(new StackNode);
			n->state = state;
			StackNode* ptr = n.get();
			this->stack_elements.push_front(std::move(n));
			ptr->source = this->stack_elements.begin();
			this->stack_top_next()->push_back(ptr);
			map[state->id] = ptr;
			return ptr;
		}

		void generate_first_sets();
		void generate_follow_sets();
		void generate_itemsets();
		void generate_closure(std::set<Item>*, std::set<Item>*);

		void reduce(std::vector<StackNode*>*, std::shared_ptr<Token>);
		void shift(std::vector<StackNode*>*, std::shared_ptr<Token>);
		void reduce_shift(StackNode*, Production*, std::shared_ptr<Match>);
		//void reduce_via_path(Path*, Token*);

		static void find_paths(std::vector<StackNode*>*, Int, StackNode*, Production*, std::vector<std::unique_ptr<StackPath>>*);
		static void find_paths(StackNode*, Int, StackNode*, Production*, std::vector<std::unique_ptr<StackPath>>*);
		static void append_paths(StackNode*, std::vector<std::unique_ptr<StackPath>>*, std::vector<std::unique_ptr<StackPath>>*);

		static void debug_production(Production*, Int);
		static void debug_item(Item);
		static void debug_set(std::set<std::string>);
		static void debug_match(Match*, Int);
	};

}

#endif /* MIDORI_GLR_H_INCLUDED */
