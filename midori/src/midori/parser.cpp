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
	this->add_production(target, symbols, handler, nullptr);
}
void Parser::add_production(std::string target, std::vector<std::string> symbols, ProductionHandler handler, RewriteHandler rewrite) {
	std::unique_ptr<Production> p(new Production);
	p->index = this->productions.size();
	p->target = target;
	p->symbols = symbols;
	p->handler = handler;
	p->rewrite = rewrite;
	this->nonterminals[target].push_back(p.get());
	this->productions.push_back(std::move(p));
}

void Parser::generate(std::string start) {
	this->terminals.insert(Lexer::TOKEN_END);
	//this->add_production(Parser::ROOT, { start, Lexer::TOKEN_END }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
	this->add_production(Parser::ROOT, { start }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		return std::move(m->value);
	});
	this->lexer.generate();
	this->generate_first_sets();
	//this->generate_follow_sets();
	//this->generate_itemsets();
	this->generate_lr1_itemsets();

	/*
	for (std::unique_ptr<ItemSet> const& i : this->states) {
		for (std::map<std::string, ItemSet*>::value_type it : i->next) {
			if (i->reductions.find(it.first) != i->reductions.end()) {
				std::cout << "shift reduce conflict at state " << i->index << " for " << it.first << std::endl;
			}
		}
	}
	*/

	for (std::unique_ptr<LR1ItemSet> const& i : this->lr1_states) {
		for (std::map<std::string, LR1ItemSet*>::value_type it : i->next) {
			if (i->reductions.find(it.first) != i->reductions.end()) {
				std::cout << "shift reduce conflict at state " << i->index << " for " << it.first << std::endl;
			}
		}
	}
}

std::unique_ptr<MatchedNonterminal> Parser::parse(IInputStream* in) {
	this->reset();
	std::cout << std::endl << "===== Parsing" << std::endl;
	//this->parse_stack.push(0);
	this->parse_stack_states.push(this->lr1_states.front().get());
	bool accept = false;
	while (true) {
		std::unique_ptr<Match> s = this->next_symbol(in);
		if (MatchedTerminal* m = dynamic_cast<MatchedTerminal*>(s.get())) {
			Token* t = m->token.get();
			std::cout << "Got token";
			for (std::string const& tag : t->tags) {
				std::cout << " " << tag;
			}
			std::cout << ": " << t->lexeme << " (" << t->loc.line << ", " << t->loc.column << ")" <<std::endl;
			if (t->tags.at(0) == Lexer::TOKEN_BAD) {
				std::cout << "Bad token " << std::endl;
				break;
			}
		}
		if (this->parse_advance(std::move(s), &accept)) {
			break;
		}
	}
	mdk::printf("[debug] parse stack size %zd, stack matches %zd\n", this->parse_stack_states.size(), this->parse_stack_matches.size());
	Parser::debug_match(this->parse_stack_matches.top().get(), 0);
	if (!accept) {
		return nullptr;
	}
	assert(this->parse_stack_states.size() == 2);
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
	while (!this->parse_stack_states.empty()) {
		this->parse_stack_states.pop();
	}
	while (!this->parse_stack_matches.empty()) {
		this->parse_stack_matches.pop();
	}
}

#pragma mark - Parser - private

bool Parser::parse_advance(std::unique_ptr<Match> s, bool* accept) {
	//mdk::printf("[debug] parse at state %d, size %zd\n", this->parse_stack.top(), this->parse_stack.size());
	std::cout << "parse at state " << this->parse_stack_states.top()->index << ", size " << this->parse_stack_states.size() << std::endl;
	if (MatchedTerminal* m = dynamic_cast<MatchedTerminal*>(s.get())) {
		for (std::string const& tag : m->token->tags) {
			std::cout << "Trying tag " << tag << std::endl;
			s = this->parse_symbol(tag, std::move(s), accept);
			if (s == nullptr) {
				return (*accept);
			}
		}
	} else if (MatchedNonterminal* m = dynamic_cast<MatchedNonterminal*>(s.get())) {
		std::cout << "Trying nonterminal " << m->production->target << std::endl;
		s = this->parse_symbol(m->production->target, std::move(s), accept);
		if (s == nullptr) {
			return (*accept);
		}
	} else {
		assert(false);
	}
	std::cout << "no rules" << std::endl;
	/*
	ItemSet* curr = this->current_state();
	std::cout << "no rules, expected to see" << std::endl;
	for (std::map<std::string, ItemSet*>::value_type const& kv : curr->next) {
		std::cout << kv.first << " -> " << kv.second->index << std::endl;
	}
	for (std::map<std::string, Production*>::value_type const& kv : curr->reductions) {
		std::cout << kv.first << " <- ";
		Parser::debug_production(kv.second);
	}
	*/
	return true;
}

std::unique_ptr<Match> Parser::parse_symbol(std::string tag, std::unique_ptr<Match> s, bool* accept) {
	//ItemSet* curr = this->current_state();
	LR1ItemSet* curr = this->parse_stack_states.top();
	std::map<std::string, LR1ItemSet*>::iterator shift = curr->next.find(tag);
	std::map<std::string, Production*>::iterator reduce = curr->reductions.find(tag);
	if (shift != curr->next.end()) {
		if (reduce != curr->reductions.end()) {
			std::cout << "shift reduce conflict" << std::endl;
		}
		/*
		if (tag == Lexer::TOKEN_END) {
			*accept = true;
			return nullptr;
		}
		*/
		std::cout << "shifting to " << shift->second->index << std::endl;
		//this->parse_stack.push(shift->second->index);
		this->parse_stack_states.push(shift->second);
		this->parse_stack_matches.push(std::move(s));
		return nullptr;
	}
	if (reduce != curr->reductions.end()) {
		if (curr->accept && tag == Lexer::TOKEN_END) {
			std::cout << "accepting" << std::endl;
			*accept = true;
			return nullptr;
		}
		this->push_symbol(std::move(s));
		std::cout << "reducing via rule ";
		Parser::debug_production(reduce->second);
		std::unique_ptr<MatchedNonterminal> mnt(new MatchedNonterminal(reduce->second));
		size_t n = reduce->second->symbols.size();
		for (size_t i = 0; i < n; i++) {
			//this->parse_stack.pop();
			this->parse_stack_states.pop();
			std::unique_ptr<Match> m = std::move(this->parse_stack_matches.top());
			this->parse_stack_matches.pop();
			mnt->terms[n - i - 1] = std::move(m);
		}
		if (reduce->second->handler != nullptr) {
			mnt->value = reduce->second->handler(mnt.get());
		}
		if (reduce->second->rewrite != nullptr) {
			std::unique_ptr<MatchedNonterminal> transformed = reduce->second->rewrite(std::move(mnt));
			this->pull_symbols(std::move(transformed));
			return nullptr;
		}
		/*
		// TODO: why no cast?
		this->parse_stack_matches.push(std::move(mnt));
		std::cout << "getting shift from state " << this->parse_stack.top() << " size " << this->parse_stack.size() << std::endl;
		ItemSet* reduce_state = this->current_state();
		std::map<std::string, ItemSet*>::iterator reduction_shift = reduce_state->next.find(reduce->second->target);
		assert(reduction_shift != reduce_state->next.end());
		std::cout << "reduction shifting to " << reduction_shift->second->index << std::endl;
		this->parse_stack.push(reduction_shift->second->index);
		this->push_token(std::move(t));
		*/
		this->push_symbol(std::move(mnt));
		return nullptr;
	}
	return s;
}

void Parser::generate_itemsets() {
	assert(this->states.size() == 0);
	std::unique_ptr<ItemSet> start(new ItemSet);
	for (Production* const p : this->nonterminals.at(Parser::ROOT)) {
		start->kernel.insert(Item(p, 0));
	}
	std::list<ItemSet*> q;
	auto register_state = [ &q ](Parser* self, std::unique_ptr<ItemSet> is) -> ItemSet* {
		is->index = self->states.size();
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
			if (Parser::item_is_done(i)) {
				for (std::string const& s : this->follows[i.first->target]) {
					std::map<std::string, Production*>::iterator it = is->reductions.find(s);
					if (it == is->reductions.end()) {
						is->reductions[s] = i.first;
					} else {
						// TODO
						// how to handle
					}
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
					next->kernel.insert(Item(i2.first, i2.second + 1));
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
			if (!Parser::item_is_done(i)) {
				std::string s = i.first->symbols.at(i.second);
				if (this->symbol_is_token(s)) {
					continue;
				}
				for (Production* const p : this->nonterminals.at(s)) {
					q.push_back(Item(p, 0));
				}
			}
		}
	}
}

/*
 * Dragon book page 261
 */
void Parser::generate_closure(LR1ItemSet* itemset) {
	std::list<LR1Item> q(itemset->kernel.begin(), itemset->kernel.end());
	while (!q.empty()) {
		LR1Item i = q.front();
		q.pop_front();
		if (itemset->closure.insert(i).second) {
			if (i.is_done()) {
				continue;
			}
			std::string s = i.production->symbols.at(i.dot);
			if (this->symbol_is_token(s)) {
				continue;
			}
			std::vector<std::string> l;
			Int j = i.dot + 1;
			while (j < i.production->symbols.size()) {
				std::string s2 = i.production->symbols.at(j);
				std::map<std::string, std::set<std::string>>::iterator it = this->firsts.find(s2);
				assert(it != this->firsts.end());
				std::set<std::string> const& f = it->second;
				l.insert(l.end(), f.begin(), f.end());
				if (this->nullable.find(s2) == this->nullable.end()) {
					break;
				}
				j++;
			}
			if (j == i.production->symbols.size()) {
				l.push_back(i.terminal);
			}
			for (Production* const p : this->nonterminals.at(s)) {
				for (std::string const& s2 : l) {
					q.emplace_back(p, 0, s2);
				}
			}
		}
	}
}

LR1ItemSet* Parser::generate_goto(LR1ItemSet* itemset, std::string symbol) {
	std::unique_ptr<LR1ItemSet> next(new LR1ItemSet);
	for (LR1Item const& i : itemset->closure) {
		if (i.is_done()) {
			continue;
		}
		next->kernel.emplace(i.production, i.dot + 1, i.terminal);
	}
	std::map<std::set<LR1Item>, LR1ItemSet*>::iterator it = this->lr1_itemsets.find(next->kernel);
	if (it == this->lr1_itemsets.end()) {
		next->index = this->lr1_states.size();
		LR1ItemSet* ptr = next.get();
		this->lr1_states.push_back(std::move(next));
		return ptr;
	}
	return it->second;
}

void Parser::generate_lr1_itemsets() {
	std::unique_ptr<LR1ItemSet> start(new LR1ItemSet);
	std::map<std::string, std::vector<Production*>>::iterator it = this->nonterminals.find(Parser::ROOT);
	assert(it != this->nonterminals.end());
	assert(it->second.size() == 1);
	start->kernel.emplace(it->second.front(), 0, Lexer::TOKEN_END);
	std::list<LR1ItemSet*> q;
	q.push_back(this->register_state(std::move(start)));
	while (!q.empty()) {
		LR1ItemSet* is = q.front();
		q.pop_front();
		for (LR1Item const& i : is->closure) {
			if (i.is_done()) {
				std::map<std::string, Production*>::iterator it = is->reductions.find(i.terminal);
				if (it == is->reductions.end()) {
					is->reductions[i.terminal] = i.production;
				} else {
					// TODO: handle this
					std::cout << "reduce reduce conflict" << std::endl;
				}
				continue;
			}
			std::string next_symbol = i.production->symbols.at(i.dot);
			std::unique_ptr<LR1ItemSet> next(new LR1ItemSet);
			for (LR1Item const& i2 : is->closure) {
				if (i2.is_done()) {
					continue;
				}
				if (i2.production->symbols.at(i2.dot) == next_symbol) {
					next->kernel.emplace(i2.production, i2.dot + 1, i2.terminal);
				}
			}
			std::map<std::set<LR1Item>, LR1ItemSet*>::iterator it = this->lr1_itemsets.find(next->kernel);
			if (it == this->lr1_itemsets.end()) {
				LR1ItemSet* canonical = this->register_state(std::move(next));
				is->next[next_symbol] = canonical;
				q.push_back(canonical);
			} else {
				is->next[next_symbol] = it->second;
			}
		}
	}
}

LR1ItemSet* Parser::register_state(std::unique_ptr<LR1ItemSet> itemset) {
	itemset->index = this->lr1_states.size();
	// why isn't this 0 initialized?
	itemset->accept = false;
	for (LR1Item const& i : itemset->kernel) {
		if (i.production->target == Parser::ROOT && i.is_done()) {
			// TODO
			if (i.terminal != Lexer::TOKEN_END) {
				std::cout << "UNEXPECTED" << std::endl;
			}
			itemset->accept = true;
		}
	}
	this->generate_closure(itemset.get());
	LR1ItemSet* ret = itemset.get();
	this->lr1_itemsets[itemset->kernel] = ret;
	this->lr1_states.push_back(std::move(itemset));
	return ret;
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

void Parser::push_symbol(std::unique_ptr<Match> s) {
	this->symbol_buffer.push(std::move(s));
}

void Parser::pull_symbols(std::unique_ptr<Match> s) {
	if (dynamic_cast<MatchedTerminal*>(s.get())) {
		this->push_symbol(std::move(s));
		return;
	}
	MatchedNonterminal* m = dynamic_cast<MatchedNonterminal*>(s.get());
	assert(m != nullptr);
	if (m->production != nullptr) {
		std::cout << "[debug] pulling already made nonterminal" << std::endl;
		this->push_symbol(std::move(s));
		return;
	}
	for (std::vector<std::unique_ptr<Match>>::reverse_iterator it = m->terms.rbegin(); it != m->terms.rend(); it++) {
		this->pull_symbols(std::move(*it));
	}
}

std::unique_ptr<Match> Parser::next_symbol(IInputStream* in) {
	if (this->symbol_buffer.empty()) {
		std::unique_ptr<Token> t = this->lexer.scan(in);
		return std::unique_ptr<Match>(new MatchedTerminal(std::move(t)));
	}
	std::unique_ptr<Match> s = std::move(this->symbol_buffer.top());
	this->symbol_buffer.pop();
	return s;
}

bool Parser::production_is_epsilon(Production* p) {
	return p->symbols.size() == 0;
}

bool Parser::item_is_done(Item item) {
	return item.second == (Int) item.first->symbols.size();
}

#pragma mark - Parser - debug
void Parser::debug_production(Production* p, Int dot, std::string terminal) {
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
	if (terminal.length() > 0) {
		std::cout << " (" << terminal << ")";
	}
	std::cout << std::endl;
}

void Parser::debug_item(Item item) {
	Parser::debug_production(item.first, (Int) item.second, "");
}

void Parser::debug_item(LR1Item item) {
	Parser::debug_production(item.production, (Int) item.dot, item.terminal);
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
	for (std::unique_ptr<LR1ItemSet> const& is : this->lr1_states) {
		std::cout << "=== LR1 Item Set " << is->index << ", accept " << is->accept << std::endl;
		std::cout << "Kernel:" << std::endl;
		for (LR1Item const& x : is->kernel) {
			std::cout << "\t";
			Parser::debug_item(x);
		}
		std::cout << "Closure:" << std::endl;
		for (LR1Item const& x : is->closure) {
			std::cout << "\t";
			Parser::debug_item(x);
		}
		std::cout << "Next states:" << std::endl;
		for (std::map<std::string, LR1ItemSet*>::value_type const& kv : is->next) {
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
