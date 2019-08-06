#include "parser.h"
#include "helper.h"
#include <utility>

std::string const Parser::ROOT = "$root";
std::string const Parser::TOKEN_MIDORI = "#";

ParserAST::~ParserAST() {
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

void Parser::generate(Type type, std::string start) {
	this->terminals.insert(Lexer::TOKEN_END);
	this->add_production(Parser::ROOT, { start }, [](MatchedNonterminal* m) -> std::unique_ptr<ParserAST> {
		return std::move(m->value);
	});
	this->lexer.generate();
	this->generate_first_sets();
	this->generate_itemsets(type);
	if (type == Type::LALR1) {
		this->lookaheads.resize(this->lr0_states.size());
		this->lookahead_propagates.resize(this->lr0_states.size());
		this->discover_lookaheads();
		this->propagate_lookaheads();
		this->generate_lalr_itemsets();
	}
	this->generate_actions();
}

std::unique_ptr<MatchedNonterminal> Parser::parse(IInputStream* in) {
	this->reset();
	std::cout << std::endl << "===== Parsing" << std::endl;
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
	mdk::printf("[debug] parse lr1 stack %zd, stack matches %zd\n", this->parse_stack_states.size(), this->parse_stack_matches.size());
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
	while (!this->parse_stack_states.empty()) {
		this->parse_stack_states.pop();
	}
	while (!this->parse_stack_matches.empty()) {
		this->parse_stack_matches.pop();
	}
}

std::vector<GrammarConflict> Parser::conflicts() {
	return this->_conflicts;
}

#pragma mark - Parser - private

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

/*
 * Dragon book page 245
 */
void Parser::generate_lr0_closure(ItemSet* is) {
	std::list<Item> q(is->kernel.begin(), is->kernel.end());
	while (!q.empty()) {
		Item i = q.front();
		q.pop_front();
		if (is->closure.insert(i).second) {
			if (!i.is_done()) {
				std::string s = i.next_symbol();
				if (this->symbol_is_token(s)) {
					continue;
				}
				for (Production* const p : this->nonterminals.at(s)) {
					q.emplace_back(p, 0, "");
				}
			}
		}
	}
}

/*
 * Dragon book page 261
 */
void Parser::generate_lr1_closure(ItemSet* is) {
	std::list<Item> q(is->kernel.begin(), is->kernel.end());
	while (!q.empty()) {
		Item i = q.front();
		q.pop_front();
		if (is->closure.insert(i).second) {
			if (i.is_done()) {
				continue;
			}
			std::string s = i.next_symbol();
			if (this->symbol_is_token(s)) {
				continue;
			}
			std::vector<std::string> l;
			Int j = i.dot + 1;
			while (j < i.production->symbols.size()) {
				std::string s2 = i.production->symbols.at(j);
				std::map<std::string, std::set<std::string>>::iterator it = this->firsts.find(s2);
				assert(it != this->firsts.end());
				l.insert(l.end(), it->second.begin(), it->second.end());
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

void Parser::generate_itemsets(Type type) {
	std::unique_ptr<ItemSet> start(new ItemSet);
	std::map<std::string, std::vector<Production*>>::iterator it = this->nonterminals.find(Parser::ROOT);
	std::vector<Production*>& v = this->nonterminals.at(Parser::ROOT);
	assert(v.size() == 1);
	std::string s = "";
	if (type == Type::LR1) {
		s = Lexer::TOKEN_END;
	}
	start->kernel.emplace(v.front(), 0, s);

	std::list<ItemSet*> q;
	this->register_state(type, std::move(start), &q);
	while (!q.empty()) {
		ItemSet* is = q.front();
		q.pop_front();
		for (Item const& i : is->closure) {
			if (i.is_done()) {
				if (type == Type::LR1) {
					is->reductions[i.terminal].push_back(i.production);
				}
				continue;
			}
			std::string next_symbol = i.next_symbol();
			std::unique_ptr<ItemSet> next(new ItemSet);
			for (Item const& i2 : is->closure) {
				if (i2.is_done()) {
					continue;
				}
				if (i2.next_symbol() == next_symbol) {
					next->kernel.emplace(i2.production, i2.dot + 1, i2.terminal);
				}
			}
			is->next[next_symbol] = this->register_state(type, std::move(next), &q);
		}
	}
}

void Parser::generate_actions() {
	for (std::unique_ptr<ItemSet> const& is : this->lr1_states) {
		for (std::map<std::string, ItemSet*>::value_type const& kv : is->next) {
			is->actions.emplace(std::piecewise_construct, std::forward_as_tuple(kv.first), std::forward_as_tuple(kv.second, nullptr));
			std::map<std::string, std::vector<Production*>>::iterator it = is->reductions.find(kv.first);
			if (it == is->reductions.end()) {
				continue;
			}
			GrammarConflict gc;
			gc.type = GrammarConflict::Type::ShiftReduce;
			gc.state = is.get();
			gc.symbol = kv.first;
			for (Item const& i : kv.second->kernel) {
				assert(i.production->symbols.at(i.dot - 1) == kv.first);
				gc.productions.push_back(i.production);
			}
			gc.productions.insert(gc.productions.end(), it->second.begin(), it->second.end());
			this->_conflicts.push_back(gc);
		}
		for (std::map<std::string, std::vector<Production*>>::value_type const& kv : is->reductions) {
			std::map<std::string, Action>::iterator it = is->actions.find(kv.first);
			if (it != is->actions.end()) {
				continue;
			}
			Production* p = kv.second.front();
			if (kv.second.size() > 1) {
				GrammarConflict gc;
				gc.type = GrammarConflict::Type::ReduceReduce;
				gc.state = is.get();
				gc.symbol = kv.first;
				gc.productions.push_back(p);
				size_t a = p->symbols.size();
				Int b = -(p->index);
				for (size_t i = 1; i < kv.second.size(); i++) {
					Production* q = kv.second.at(i);
					gc.productions.push_back(q);
					// prioritize longer rules, then rules added earlier (lower index)
					size_t c = q->symbols.size();
					Int d = -(q->index);
					if (std::tie(c, d) > std::tie(a, b)) {
						p = q;
						a = c;
						b = d;
					}
				}
				this->_conflicts.push_back(gc);
			}
			is->actions.emplace(std::piecewise_construct, std::forward_as_tuple(kv.first), std::forward_as_tuple(nullptr, p));
		}
	}
}

ItemSet* Parser::register_state(Type type, std::unique_ptr<ItemSet> is, std::list<ItemSet*>* queue) {
	ItemSet* ptr = is.get();
	is->accept = false;
	for (Item const& i : is->kernel) {
		if ((i.production->target == Parser::ROOT) && i.is_done()) {
			if (type == Type::LR1) {
				assert(i.terminal == Lexer::TOKEN_END);
			}
			is->accept = true;
		}
	}
	std::vector<std::unique_ptr<ItemSet>>* v = nullptr;
	std::map<std::set<Item>, ItemSet*>* m = nullptr;
	if (type == Type::LALR1) {
		v = &(this->lr0_states);
		m = &(this->lr0_itemsets);
		this->generate_lr0_closure(ptr);
	} else if (type == Type::LR1) {
		v = &(this->lr1_states);
		m = &(this->lr1_itemsets);
		this->generate_lr1_closure(ptr);
	}
	std::map<std::set<Item>, ItemSet*>::iterator it = m->find(is->kernel);
	if (it == m->end()) {
		is->index = v->size();
		queue->push_back(ptr);
		m->operator[](is->kernel) = ptr;
		v->push_back(std::move(is));
		return ptr;
	}
	return it->second;
}

void Parser::discover_lookaheads() {
	ItemSet* root = this->lr0_states.at(0).get();
	assert(root->kernel.size() == 1);
	Item start = *(root->kernel.begin());
	std::unique_ptr<std::set<std::string>> s(new std::set<std::string>);
	s->insert(Lexer::TOKEN_END);
	this->lookaheads.at(0)[start] = std::move(s);
	for (std::unique_ptr<ItemSet> const& is : this->lr0_states) {
		for (Item const& i : is->kernel) {
			std::cout << "discovering lookaheads for state " << is->index << " kernel item ";
			Parser::debug_item(i);
			std::unique_ptr<ItemSet> js(new ItemSet);
			js->kernel.emplace(i.production, i.dot, Parser::TOKEN_MIDORI);
			this->generate_lr1_closure(js.get());
			for (Item const& j : js->closure) {
				std::cout << "\tLR1Item in closure: ";
				Parser::debug_item(j);
				if (j.is_done()) {
					continue;
				}
				std::string s = j.production->symbols.at(j.dot);
				Item i2(j.production, j.dot + 1, "");
				ItemSet* next = is->next.at(s);
				std::cout << "\tgetting from next state " << next->index << " item ";
				Parser::debug_item(i2);
				assert(next->kernel.count(i2) == 1);
				LookaheadMap& m = this->lookaheads.at(next->index);
				LookaheadMap::iterator it = m.find(i2);
				std::set<std::string>* x = nullptr;
				if (it == m.end()) {
					std::unique_ptr<std::set<std::string>> x2(new std::set<std::string>);
					x = x2.get();
					m[i2] = std::move(x2);
				} else {
					x = it->second.get();
				}
				if (j.terminal == Parser::TOKEN_MIDORI) {
					std::cout << "propagating from ";
					Parser::debug_item(i);
					std::cout << "\tto ";
					Parser::debug_item(i2);
					std::cout << "\tin state " << next->index << std::endl;
					this->lookahead_propagates.at(is->index)[i].push_back(x);
				} else {
					std::cout << "inserting " << j.terminal << " for state " << next->index << " kernel item ";
					Parser::debug_item(i2);
					x->insert(j.terminal);
				}
			}
		}
	}
}

void Parser::propagate_lookaheads() {
	bool changed = true;
	while (changed) {
		changed = false;
		for (std::unique_ptr<ItemSet> const& is : this->lr0_states) {
			for (Item const& i : is->kernel) {
				std::cout << "Propagating for ";
				Parser::debug_item(i);
				LookaheadMap& m = this->lookaheads.at(is->index);
				LookaheadMap::iterator it = m.find(i);
				if (it == m.end()) {
					std::cout << "\tno entries" << std::endl;
					continue;
				}
				std::set<std::string>* from = it->second.get();
				std::cout << "\thave " << from->size() << " entries to propagate to " << this->lookahead_propagates.at(is->index)[i].size() << " others" << std::endl;
				for (std::set<std::string>* const to : this->lookahead_propagates.at(is->index)[i]) {
					size_t old = to->size();
					to->insert(from->begin(), from->end());
					changed = changed || (to->size() != old);
				}
				std::cout << "done" << std::endl;
			}
		}
	}
}

void Parser::generate_lalr_itemsets() {
	for (std::unique_ptr<ItemSet> const& is : this->lr0_states) {
		std::unique_ptr<ItemSet> js(new ItemSet);
		js->index = is->index;
		js->accept = is->accept;
		LookaheadMap& m = this->lookaheads.at(is->index);
		for (Item const& i : is->kernel) {
			LookaheadMap::iterator it = m.find(i);
			assert(it != m.end());
			for (std::string const& s : (*(it->second))) {
				js->kernel.emplace(i.production, i.dot, s);
			}
		}
		this->generate_lr1_closure(js.get());
		this->lr1_states.push_back(std::move(js));
	}
	for (std::unique_ptr<ItemSet> const& is : this->lr0_states) {
		ItemSet* js = this->lr1_states.at(is->index).get();
		for (std::map<std::string, ItemSet*>::value_type const& kv : is->next) {
			js->next[kv.first] = this->lr1_states.at(kv.second->index).get();
		}
		for (Item const& j : js->closure) {
			if (j.is_done()) {
				js->reductions[j.terminal].push_back(j.production);
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

bool Parser::parse_advance(std::unique_ptr<Match> s, bool* accept) {
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
	std::cout << "no rules, expected to see" << std::endl;
	ItemSet* curr = this->parse_stack_states.top();
	for (std::map<std::string, Action>::value_type const& kv : curr->actions) {
		std::cout << kv.first;
		if (kv.second.shift != nullptr) {
			assert(kv.second.reduce == nullptr);
			std::cout << " -> " << kv.second.shift->index << std::endl;
		} else {
			assert(kv.second.reduce != nullptr);
			std::cout << " <- ";
			Parser::debug_production(kv.second.reduce);
		}
	}
	return true;
}

std::unique_ptr<Match> Parser::parse_symbol(std::string tag, std::unique_ptr<Match> s, bool* accept) {
	ItemSet* curr = this->parse_stack_states.top();
	std::map<std::string, Action>::iterator it = curr->actions.find(tag);
	if (it == curr->actions.end()) {
		return s;
	}
	ItemSet* shift = it->second.shift;
	Production* reduce = it->second.reduce;
	if (shift != nullptr) {
		assert(reduce == nullptr);
		std::cout << "shifting to " << shift->index << std::endl;
		this->parse_stack_states.push(shift);
		this->parse_stack_matches.push(std::move(s));
		return nullptr;
	}
	assert(reduce != nullptr);
	if (curr->accept && tag == Lexer::TOKEN_END) {
		std::cout << "accepting" << std::endl;
		*accept = true;
		return nullptr;
	}
	this->push_symbol(std::move(s));
	std::cout << "reducing via rule ";
	Parser::debug_production(reduce);
	std::unique_ptr<MatchedNonterminal> mnt(new MatchedNonterminal(reduce));
	size_t n = reduce->symbols.size();
	for (size_t i = 0; i < n; i++) {
		this->parse_stack_states.pop();
		std::unique_ptr<Match> m = std::move(this->parse_stack_matches.top());
		this->parse_stack_matches.pop();
		mnt->terms[n - i - 1] = std::move(m);
	}
	if (reduce->handler != nullptr) {
		mnt->value = reduce->handler(mnt.get());
	}
	if (reduce->rewrite != nullptr) {
		std::unique_ptr<MatchedNonterminal> transformed = reduce->rewrite(std::move(mnt));
		// TODO: why no cast?
		this->pull_symbols(std::move(transformed));
		return nullptr;
	}
	this->push_symbol(std::move(mnt));
	return nullptr;
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
	for (std::unique_ptr<ItemSet> const& is : this->lr0_states) {
		std::cout << "=== Item Set " << is->index << std::endl;
		std::cout << "Kernel:" << std::endl;
		for (Item const& x : is->kernel) {
			std::cout << "\t";
			Parser::debug_item(x);
			std::cout << "\t\tLookaheads:" << std::endl;
			std::cout << "\t\t";
			LookaheadMap& m = this->lookaheads.at(is->index);
			LookaheadMap::iterator it = m.find(x);
			if (it == m.end()) {
				std::cout << "(none)";
				std::cout << std::endl;
			} else {
				Parser::debug_set(*(it->second));
			}
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
		for (std::map<std::string, std::vector<Production*>>::value_type const& kv : is->reductions) {
			for (Production* const p : kv.second) {
				std::cout << "\t" << kv.first << " -> ";
				Parser::debug_production(p);
			}
		}
		std::cout << "=== done " << is->index << std::endl;
		std::cout << std::endl;
	}
	for (std::unique_ptr<ItemSet> const& is : this->lr1_states) {
		std::cout << "=== LR1 Item Set " << is->index << ", accept " << is->accept << std::endl;
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
		for (std::map<std::string, std::vector<Production*>>::value_type const& kv : is->reductions) {
			for (Production* const p : kv.second) {
				std::cout << "\t" << kv.first << " -> ";
				Parser::debug_production(p);
			}
		}
		std::cout << "=== done " << is->index << std::endl;
		std::cout << std::endl;
	}
}
