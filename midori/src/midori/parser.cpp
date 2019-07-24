#include "parser.h"
#include "helper.h"
#include <list>

std::string const Parser::ROOT = "$root";

ParserAST::~ParserAST() {
	return;
}

ParserASTString::ParserASTString(std::string s) : str(s) {
	return;
}

Match::~Match() {
	return;
}
MatchedTerminal::MatchedTerminal(std::unique_ptr<Token> t) : token(std::move(t)) {
	return;
}
MatchedNonterminal::MatchedNonterminal() : MatchedNonterminal(nullptr, 0) {
	return;
}
MatchedNonterminal::MatchedNonterminal(Production* p) : MatchedNonterminal(p, p->symbols.size()) {
	return;
}
MatchedNonterminal::MatchedNonterminal(Production* p, size_t n) : production(p) , terms(n), value(nullptr) {
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
	std::unique_ptr<Production> p(new Production);
	p->target = target;
	p->symbols = symbols;
	p->handler = handler;
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
}

std::unique_ptr<MatchedNonterminal> Parser::parse(IInputStream* in) {
	this->reset();
	std::cout << std::endl << "===== Parsing" << std::endl;
	this->parse_stack.push(0);
	bool accept = false;
	while (true) {
		std::unique_ptr<Token> t = this->next_token(in);
		std::cout << "Got token";
		for (std::string const& tag : t->tags) {
			std::cout << " " << tag;
		}
		std::cout << ": " << t->lexeme << " (" << t->loc.line << ", " << t->loc.column << ")" <<std::endl;
		if (t->tags.at(0) == Lexer::TOKEN_BAD) {
			std::cout << "Bad token " << std::endl;
			break;
		}
		if (this->parse_advance(std::move(t), &accept)) {
			break;
		}
	}
	mdk::printf("[debug] parse stack size %zd, stack matches %zd\n", this->parse_stack.size(), this->parse_stack_matches.size());
	Parser::debug_match(this->parse_stack_matches.top().get(), 0);
	if (!accept) {
		return nullptr;
	}
	assert(this->parse_stack.size() == 2);
	assert(this->parse_stack_matches.size() == 1);
	std::unique_ptr<Match> m = std::move(this->parse_stack_matches.top());
	this->parse_stack_matches.pop();
	std::unique_ptr<MatchedNonterminal> ret(dynamic_cast<MatchedNonterminal*>(m.release()));
	return ret;
}

void Parser::reset() {
	this->lexer.reset();
	while (!this->parse_stack.empty()) {
		this->parse_stack.pop();
	}
	while (!this->parse_stack_matches.empty()) {
		this->parse_stack_matches.pop();
	}
}

#pragma mark - Parser - private

bool Parser::parse_advance(std::unique_ptr<Token> t, bool* accept) {
	mdk::printf("[debug] parse at state %d, size %zd\n", this->parse_stack.top(), this->parse_stack.size());
	ItemSet* curr = this->current_state();
	for (std::string const& tag : t->tags) {
		std::cout << "Trying tag " << tag << std::endl;
		std::map<std::string, ItemSet*>::iterator next_shift = curr->next.find(tag);
		std::map<std::string, Production*>::iterator next_reduction = curr->reductions.find(tag);
		if (next_shift != curr->next.end()) {
			if (next_reduction != curr->reductions.end()) {
				std::cout << "Shift reduce conflict" << std::endl;
			}
			if (tag == Lexer::TOKEN_END) {
				*accept = true;
				return true;
			}
			std::cout << "shifting to " << next_shift->second->index << std::endl;
			this->parse_stack.push(next_shift->second->index);
			this->parse_stack_matches.push(std::unique_ptr<Match>(new MatchedTerminal(std::move(t))));
			return false;
		}
		if (next_reduction != curr->reductions.end()) {
			std::cout << "reducing via rule ";
			Parser::debug_production(next_reduction->second);
			std::unique_ptr<MatchedNonterminal> mnt(new MatchedNonterminal(next_reduction->second));
			for (size_t i = 0; i < next_reduction->second->symbols.size(); i++) {
				this->parse_stack.pop();
				std::unique_ptr<Match> m = std::move(this->parse_stack_matches.top());
				this->parse_stack_matches.pop();
				mnt->terms[next_reduction->second->symbols.size() - i - 1] = std::move(m);
			}
			if (next_reduction->second->handler != nullptr) {
				mnt->value = next_reduction->second->handler(mnt.get());
			}
			// TODO: why no cast?
			this->parse_stack_matches.push(std::move(mnt));
			std::cout << "getting shift from state " << this->parse_stack.top() << " size " << this->parse_stack.size() << std::endl;
			ItemSet* reduce_state = this->current_state();
			std::map<std::string, ItemSet*>::iterator reduction_shift = reduce_state->next.find(next_reduction->second->target);
			assert(reduction_shift != reduce_state->next.end());
			std::cout << "reduction shifting to " << reduction_shift->second->index << std::endl;
			this->parse_stack.push(reduction_shift->second->index);
			this->push_token(std::move(t));
			return false;
		}
	}
	std::cout << "no rules, expected to see" << std::endl;
	for (std::map<std::string, ItemSet*>::value_type const& kv : curr->next) {
		std::cout << kv.first << " -> " << kv.second->index << std::endl;
	}
	for (std::map<std::string, Production*>::value_type const& kv : curr->reductions) {
		std::cout << kv.first << " <- ";
		Parser::debug_production(kv.second);
	}
	return true;
}

void Parser::generate_itemsets() {
	assert(this->states.size() == 0);
	std::unique_ptr<ItemSet> start(new ItemSet);
	for (Production* p : this->nonterminals.at(Parser::ROOT)) {
		start->head.insert(Item(p, 0));
	}
	std::list<ItemSet*> q;
	auto register_state = [ &q ](Parser* self, std::unique_ptr<ItemSet> is) -> ItemSet* {
		is->index = self->states.size();
		self->generate_closure(&(is->head), &(is->closure));
		ItemSet* ret = is.get();
		q.push_back(ret);
		self->itemsets[is->head] = ret;
		self->states.push_back(std::move(is));
		return ret;
	};
	register_state(this, std::move(start));
	while (!q.empty()) {
		ItemSet* is = q.front();
		q.pop_front();
		for (Item const& i : is->closure) {
			if (Parser::item_is_done(i)) {
				for (std::string const& s : this->follows[i.first->target]) {
					is->reductions[s] = i.first;
				}
				continue;
			}
			std::string next_symbol = i.first->symbols.at(i.second);
			std::unique_ptr<ItemSet> next(new ItemSet);
			for (Item const& i2 : is->closure) {
				if (Parser::item_is_done(i2)) {
					continue;
				}
				if (i2.first->symbols.at(i2.second) == next_symbol) {
					next->head.insert(Item(i2.first, i2.second + 1));
				}
			}
			std::map<std::set<Item>, ItemSet*>::iterator it = this->itemsets.find(next->head);
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
			if (!Parser::item_is_done(i)) {
				std::string s = i.first->symbols.at(i.second);
				if (this->symbol_is_token(s)) {
					continue;
				}
				for (Production* p : this->nonterminals.at(s)) {
					q.push_back(Item(p, 0));
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
			mdk::printf("[debug] generating first set for\n");
			Parser::debug_production(p.get());
			std::set<std::string>& f = this->firsts[p->target];
			if (Parser::production_is_epsilon(p.get())) {
				//changed = changed || f.insert(Parser::EPSILON).second;
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
				/*
				bool nullable = false;
				for (std::string const& s2 : f2) {
					if (Parser::symbol_is_epsilon(s2)) {
						nullable = true;
					} else {
						f.insert(s2);
					}
				}
				if (!nullable) {
					break;
				}
				*/
				i++;
			}
			if (i == p->symbols.size()) {
				//f.insert(Parser::EPSILON);
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
			if (Parser::production_is_epsilon(p.get())) {
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

void Parser::push_token(std::unique_ptr<Token> t) {
	this->token_buffer.push(std::move(t));
}

std::unique_ptr<Token> Parser::next_token(IInputStream* in) {
	if (this->token_buffer.empty()) {
		return this->lexer.scan(in);
	}
	std::unique_ptr<Token> t = std::move(this->token_buffer.top());
	this->token_buffer.pop();
	return t;
}

bool Parser::production_is_epsilon(Production* p) {
	return p->symbols.size() == 0;
}

bool Parser::item_is_done(Item item) {
	return item.second == (Int) item.first->symbols.size();
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

void Parser::debug_item(Item item) {
	Parser::debug_production(item.first, (Int) item.second);
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
		Parser::debug_production(mnt->production);
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
		std::cout << "=== Item Set " << is->index << std::endl;
		std::cout << "Head:" << std::endl;
		for (Item const& x : is->head) {
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
			std::cout << "\t" << kv.first << " -> " << kv.second->index << std::endl;
		}
		std::cout << "Reductions:" << std::endl;
		for (std::map<std::string, Production*>::value_type const& kv : is->reductions) {
			std::cout << "\t" << kv.first << " -> ";
			Parser::debug_production(kv.second);
		}
		std::cout << "=== done " << is->index << std::endl;
		std::cout << std::endl;
	}
}
