#include "glr.h"
#include "helper.h"
#include <list>

namespace GLR {

	std::string const Parser::ROOT = "$root";

	ParserAST::~ParserAST() {
		return;
	}

	Match::~Match() {
		return;
	}
	MatchedTerminal::MatchedTerminal(std::string s, std::unique_ptr<Token> t) : tag(s), token(std::move(t)) {
		return;
	}
	MatchedNonterminal::MatchedNonterminal(Production* p) : production(p), terms(p == nullptr ? 0 : p->symbols.size()), value(nullptr) {
		return;
	}

	void Parser::add_token(std::string tag, std::unique_ptr<RegexAST> regex) {
		this->lexer.add_rule(tag, std::move(regex));
		this->terminals.insert(tag);
	}

	void Parser::add_skip(std::string symbol) {
		this->lexer.add_skip(symbol);
	}

	void Parser::add_production(std::string target, std::vector<std::string> symbols, ProductionHandler handler) {
		this->add_production(target, symbols, handler, nullptr);
	}
	void Parser::add_production(std::string target, std::vector<std::string> symbols, ProductionHandler handler, RewriteHandler rewrite) {
		std::unique_ptr<Production> p(new Production);
		p->id = this->productions.size();
		p->target = target;
		p->symbols = symbols;
		p->handler = handler;
		p->rewrite = rewrite;
		this->nonterminals[target].push_back(p.get());
		this->productions.push_back(std::move(p));
	}

	void Parser::generate(std::string start) {
		this->terminals.insert(Lexer::TOKEN_END);
		this->add_production(Parser::ROOT, { start, Lexer::TOKEN_END }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
			return std::move(m->value);
		});
		this->lexer.generate();
		this->generate_first_sets();
		this->generate_follow_sets();
		this->generate_itemsets();
		size_t n = this->states.size();
		this->stack_top_map.resize(n);
	}

	std::unique_ptr<MatchedNonterminal> Parser::parse(IInputStream* in) {
		this->push_state(this->states.at(0).get());
		this->stack_top_index = this->stack_top_index ^ 1;
		while (true) {
			std::shared_ptr<Token> t(this->lexer.scan(in).release());
			std::vector<StackNode*>* top = this->stack_top_current();
			this->reduce(top, t);
			this->shift(top, t);
			top->clear();
			for (StackNode* const n : (*top)) {
				this->stack_top_map[n->state->id] = nullptr;
			}
			this->stack_top_index = this->stack_top_index ^ 1;
		}
		return nullptr;
	}

	void Parser::reset() {
		this->lexer.reset();
	}

#pragma mark - Parser - private

	void Parser::reduce(std::vector<StackNode*>* top, std::shared_ptr<Token> t) {
		std::vector<std::unique_ptr<StackPath>> paths;
		for (StackNode* const n : (*top)) {
			for (std::string const& s : t->tags) {
				for (Production* const prod : n->state->reductions.at(s)) {
					this->find_paths(n, prod->symbols.size(), nullptr, prod, &paths);
				}
			}
		}
		for (std::unique_ptr<StackPath> const& path : paths) {
			Production* prod = path->production;
			assert(path->nodes.size() == prod->symbols.size());
			std::shared_ptr<MatchedNonterminal> m(new MatchedNonterminal(prod));
			Int i = 0;
			for (StackNode* n : path->nodes) {
				m->terms[i] = n->value;
			}
			if (prod->handler != nullptr) {
				m->value = prod->handler(m.get());
			}
			// TODO: rewrite rules
			if (prod->is_epsilon()) {
				this->reduce_shift(path->root, prod, m);
			} else {
				for (StackNode* n : path->nodes.front()->left_siblings) {
					this->reduce_shift(n, prod, m);
				}
			}
		}
	}

	void Parser::shift(std::vector<StackNode*>* top, std::shared_ptr<Token> t) {
		for (StackNode* const n : (*top)) {
			for (std::string const& s : t->tags) {
				std::map<std::string, ItemSet*>::iterator it = n->state->next.find(s);
				if (it == n->state->next.end()) {
					continue;
				}
				StackNode* next = this->push_state(it->second);
				std::shared_ptr<Match> m(new MatchedTerminal(s, t));
				next->value = std::move(m);
				n->right_siblings.push_back(next);
				next->left_siblings.push_back(n);
			}
		}
	}

	void Parser::reduce_shift(StackNode* node, Production* prod, std::shared_ptr<Match> match) {
		std::map<std::string, ItemSet*>::iterator it = node->state->next.find(prod->target);
		assert(it != node->state->next.end());
		StackNode* next = this->push_state(it->second);
		next->value = match;
		node->right_siblings.push_back(next);
		next->left_siblings.push_back(node);
	}

	void Parser::find_paths(std::vector<StackNode*>* nodes, Int depth, StackNode* root, Production* prod, std::vector<std::unique_ptr<StackPath>>* paths) {
		for (StackNode* const n : (*nodes)) {
			std::vector<std::unique_ptr<StackPath>> sub_paths;
			Parser::find_paths(n, depth, root == nullptr ? n : root, prod, &sub_paths);
			Parser::append_paths(n, &sub_paths, paths);
		}
	}

	void Parser::find_paths(StackNode* node, Int depth, StackNode* root, Production* prod, std::vector<std::unique_ptr<StackPath>>* paths) {
		if (depth == 0) {
			std::unique_ptr<StackPath> p(new StackPath);
			p->root = root;
			p->production = prod;
			paths->push_back(std::move(p));
			return;
		}
		std::vector<std::unique_ptr<StackPath>> sub_paths;
		Parser::find_paths(&(node->left_siblings), depth - 1, root, prod, &sub_paths);
		Parser::append_paths(node, &sub_paths, paths);
	}

	void Parser::append_paths(StackNode* node, std::vector<std::unique_ptr<StackPath>>* sub_paths, std::vector<std::unique_ptr<StackPath>>* paths) {
		for (std::unique_ptr<StackPath>& p : (*sub_paths)) {
			p->nodes.push_back(node);
			paths->push_back(std::move(p));
		}
	}

	void Parser::generate_itemsets() {
		assert(this->states.size() == 0);
		std::unique_ptr<ItemSet> start(new ItemSet);
		for (Production* const p : this->nonterminals.at(Parser::ROOT)) {
			start->kernel.emplace(p, 0);
		}
		std::list<ItemSet*> q;
		auto register_state = [ &q ](Parser* self, std::unique_ptr<ItemSet> is) -> ItemSet* {
			is->id = self->states.size();
			self->generate_closure(&(is->kernel), &(is->closure));
			ItemSet* ret = is.get();
			q.push_back(ret);
			self->itemsets[is->kernel] = ret;
			self->states.push_back(std::move(is));
			return ret;
		};
		register_state(this, std::move(start));
		while (!q.empty()) {
			ItemSet* is = q.front();
			q.pop_front();
			for (Item const& i : is->closure) {
				if (i.is_done()) {
					for (std::string const& s : this->follows[i.production->target]) {
						is->reductions[s].push_back(i.production);
					}
					continue;
				}
				std::string next_symbol = i.production->symbols.at(i.dot);
				std::unique_ptr<ItemSet> next(new ItemSet);
				for (Item const& i2 : is->closure) {
					if (i2.is_done()) {
						continue;
					}
					if (i2.production->symbols.at(i2.dot) == next_symbol) {
						next->kernel.emplace(i2.production, i2.dot + 1);
					}
				}
				std::map<std::set<Item>, ItemSet*>::iterator it = this->itemsets.find(next->kernel);
				if (it == this->itemsets.end()) {
					is->next[next_symbol] = register_state(this, std::move(next));
				} else {
					is->next[next_symbol] = it->second;
				}
			}
		}
	}

	void Parser::generate_closure(std::set<Item>* kernel, std::set<Item>* closure) {
		std::list<Item> q(kernel->begin(), kernel->end());
		while (!q.empty()) {
			Item i = q.front();
			q.pop_front();
			if (closure->insert(i).second) {
				if (!i.is_done()) {
					std::string s = i.production->symbols.at(i.dot);
					if (this->terminals.find(s) != this->terminals.end()) {
						continue;
					}
					for (Production* const p : this->nonterminals.at(s)) {
						q.emplace_back(p, 0);
					}
				}
			}
		}
	}

	/*
	 * Dragon book page 221
	 */
	void Parser::generate_first_sets() {
		for (std::string const& s : this->terminals) {
			this->firsts[s].insert(s);
		}
		bool changed = true;
		while (changed) {
			changed = false;
			for (const std::unique_ptr<Production>& p : this->productions) {
				std::set<std::string>& f = this->firsts[p->target];
				if (p->is_epsilon()) {
					changed = changed || this->nullable.insert(p->target).second;
					continue;
				}
				size_t old = f.size();
				size_t i = 0;
				for (std::string const& s : p->symbols) {
					std::set<std::string>& f2 = this->firsts[s];
					f.insert(f2.begin(), f2.end());
					if (this->nullable.find(s) == this->nullable.end()) {
						break;
					}
					i++;
				}
				if (i == p->symbols.size()) {
					changed = changed || this->nullable.insert(p->target).second;
				}
				changed = changed || (f.size() != old);
			}
		}
	}

	void Parser::generate_follow_sets() {
		bool changed = true;
		while (changed) {
			changed = false;
			for (std::unique_ptr<Production> const& p : this->productions) {
				if (p->is_epsilon()) {
					continue;
				}
				std::set<std::string>& f = this->follows[p->target];
				bool epsilon = true;
				size_t i = p->symbols.size();
				for (std::vector<std::string>::reverse_iterator it = p->symbols.rbegin(); it != p->symbols.rend(); it++) {
					i--;
					if (epsilon) {
						std::set<std::string>& f2 = this->follows[*it];
						size_t old = f2.size();
						f2.insert(f.begin(), f.end());
						changed = changed || (f2.size() != old);
					}
					if (i > 0) {
						std::set<std::string>& f3 = this->follows[p->symbols.at(i - 1)];
						for (size_t j = i; j < p->symbols.size(); j++) {
							std::string s = p->symbols.at(j);
							for (std::string const& s2 : this->firsts[s]) {
								//if (!Parser::symbol_is_epsilon(s)) {
									changed = changed || f3.insert(s2).second;
								//}
							}
							if (this->nullable.find(s) == this->nullable.end()) {
								break;
							}
						}
					}
					epsilon = epsilon && (this->nullable.find(*it) != this->nullable.end());
				}
			}
		}
	}

#pragma mark - Parser - debug

	void Parser::debug_production(Production* p, Int dot) {
		std::cout << p->target << " ::=";
		Int i = 0;
		for (std::string const& s : p->symbols) {
			if (i == dot) {
				std::cout << " .";
			}
			std::cout << " " << s;
			i++;
		}
		if (i == 0) {
			std::cout << " (epsilon)";
		}
		if (i == dot) {
			std::cout << " .";
		}
		std::cout << std::endl;
	}

	void Parser::debug_item(Item i) {
		Parser::debug_production(i.production, i.dot);
	}

	void Parser::debug_set(std::set<std::string> s) {
		std::cout << "{";
		for (std::string const& str : s) {
			std::cout << " " << str;
		}
		std::cout << " }" << std::endl;
	}

	void Parser::debug_match(Match* m, Int levels) {
		std::string indent(levels * 2, ' ');
		std::cout << indent;
		if (MatchedTerminal* mt = dynamic_cast<MatchedTerminal*>(m)) {
			std::cout << "{ matched terminal";
			for (std::string const& tag : mt->token->tags) {
				std::cout << " " << tag;
			}
			std::cout << ": " << mt->token->lexeme << " }" << std::endl;
		} else if (MatchedNonterminal* mnt = dynamic_cast<MatchedNonterminal*>(m)) {
			std::cout << "{ matched nonterminal ";
			Parser::debug_production(mnt->production, -1);
			for (std::unique_ptr<Match>& t : mnt->terms) {
				Parser::debug_match(t.get(), levels + 1);
			}
			std::cout << indent << "}" << std::endl;
		} else if (m == nullptr) {
			std::cout << "{ match null }" << std::endl;
		} else {
			std::cout << "Badness in debug match: " << m << std::endl;
		}
	}

	void Parser::debug() {
		std::cout << "===== Hmmmmm" << std::endl;
		this->lexer.debug();
		std::cout << "=== Firsts" << std::endl;
		for (std::map<std::string, std::set<std::string>>::value_type const& kv : this->firsts) {
			std::cout << "\t" << kv.first << ": ";
			Parser::debug_set(kv.second);
		}
		std::cout << "=== Follows" << std::endl;
		for (std::map<std::string, std::set<std::string>>::value_type const& kv : this->follows) {
			std::cout << "\t" << kv.first << ": ";
			Parser::debug_set(kv.second);
		}
		for (std::unique_ptr<ItemSet> const& is : this->states) {
			std::cout << "=== Item Set " << is->id << std::endl;
			std::cout << "Kernel:" << std::endl;
			for (Item const& x : is->kernel) {
				std::cout << "\t";
				Parser::debug_item(x);
			}
			std::cout << "Closure:" << std::endl;
			for (Item const& x : is->closure) {
				std::cout << "\t";
				Parser::debug_item(x);
			}
			std::cout << "Next states:" << std::endl;
			for (std::map<std::string, ItemSet*>::value_type const& kv : is->next) {
				std::cout << "\t" << kv.first << " -> " << kv.second->id << std::endl;
			}
			std::cout << "Reductions:" << std::endl;
			for (std::map<std::string, std::vector<Production*>>::value_type const& kv : is->reductions) {
				std::cout << "\t" << kv.first << " -> ";
				for (Production* const p : kv.second) {
					Parser::debug_production(p, -1);
				}
			}
			std::cout << "=== done " << is->id << std::endl;
			std::cout << std::endl;
		}
	}
}
