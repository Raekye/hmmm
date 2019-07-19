#include "parser.h"
#include "helper.h"

/*
 * TODO
 * - for loops const and auto and by reference
 */

const std::string Parser::END = "$";
const std::string Parser::EPSILON = "0";

ParserAST::~ParserAST() {
	return;
}

Match::~Match() {
	return;
}
MatchedTerminal::MatchedTerminal(std::unique_ptr<Token> t) : token(std::move(t)) {
	return;
}
MatchedNonterminal::MatchedNonterminal(Production* p) : production(p), terms(p->symbols.size()), value(nullptr) {
	return;
}

void Parser::add_token(std::string tag, std::unique_ptr<RegexAST> regex) {
	this->lexer.add_rule(tag, std::move(regex));
	this->terminals.insert(tag);
}

void Parser::add_production(std::string target, std::vector<std::string> symbols, ProductionHandler handler) {
	std::unique_ptr<Production> p(new Production);
	p->target = target;
	p->symbols = symbols.empty() ? std::vector<Symbol>{ Parser::EPSILON } : symbols;
	p->handler = handler;
	this->nonterminals[target].push_back(p.get());
	this->productions.push_back(std::move(p));
}

void Parser::generate(std::string symbol) {
	this->start = symbol;
	std::set<Item> head;
	for (Production* p : this->nonterminals.at(symbol)) {
		head.insert(Item(p, 0));
	}
	this->generate_itemset(head);

	std::cout << std::endl;

	std::cout << "=== Generating extended grammar" << std::endl;
	this->generate_extended_grammar();
	std::cout << "=== Done extended grammar" << std::endl;
	std::cout << std::endl;

	std::cout << "=== Generating extended sets" << std::endl;
	this->generate_extended_first_sets();
	this->generate_extended_follow_sets();
	std::cout << "=== Done extended sets" << std::endl;
	std::cout << std::endl;

	this->generate_reductions();

	Parser::debug(this);
}

// TODO better return type possible?
std::unique_ptr<Match> Parser::parse(IInputStream* in) {
	this->reset();
	std::cout << std::endl << "===== Parsing" << std::endl;
	this->parse_stack.push(0);
	Symbol last_reduction = "";
	while (true) {
		mdk::printf("[debug] parse at state %d\n", this->parse_stack.top());
		std::unique_ptr<ItemSet>& current_state = this->states.at(this->parse_stack.top());
		bool accept = false;
		for (const Item& item : current_state->head) {
			mdk::printf("[debug] checking head ");
			Parser::debug_item(item);
			if (item.first->target == this->start && Parser::item_is_done(item)) {
				accept = true;
				break;
			}
		}
		if (accept) {
			std::cout << "Accepted." << std::endl;
			//break;
		}

		mdk::printf("[debug] last reduction is {%s}\n", last_reduction.c_str());
		Symbol lr2 = last_reduction;
		last_reduction = "";
		if (lr2.length() > 0) {
			std::map<std::string, ItemSet*>::iterator it = current_state->next.find(lr2);
			if (it != current_state->next.end()) {
				std::cout << "goto " << it->second->index << std::endl;
				this->parse_stack.push(it->second->index);
				continue;
			} else {
				// TODO: what?
				std::cout << "No next state" << std::endl;
				break;
			}
		}

		std::unique_ptr<Token> t = this->next_token(in);
		if (t == nullptr) {
			std::cout << "Got null token, state " << current_state->index << std::endl;
			// TODO: refactor?
			std::map<Symbol, Production*> reduction_row = this->reductions.at(current_state->index);
			if (reduction_row.find(Parser::END) != reduction_row.end()) {
				std::cout << "Reducing via end" << std::endl;
				t.reset(new Token(Parser::END, "", LocationInfo(0, 0)));
			} else {
				if (current_state->next.find(Parser::EPSILON) != current_state->next.end()) {
					mdk::printf("[debug] Reducing via epsilon\n");
					t.reset(new Token(Parser::EPSILON, "", LocationInfo(0, 0)));
				} else {
					std::cout << "Breaking." << std::endl;
					break;
				}
			}
		}
		std::cout << "Got token " << t->tag << ": " << t->lexeme << std::endl;
		std::map<std::string, ItemSet*>::iterator it = current_state->next.find(t->tag);
		if (it != current_state->next.end()) {
			std::cout << "shift " << it->second->index << std::endl;
			this->parse_stack.push(it->second->index);
			this->parse_stack_matches.push(std::unique_ptr<Match>(new MatchedTerminal(std::move(t))));
			continue;
		}
		std::map<Symbol, Production*> reduction_row = this->reductions.at(current_state->index);
		mdk::printf("[debug] getting reductions at %d, tag is %s\n", current_state->index, t->tag.c_str());
		std::map<Symbol, Production*>::iterator it2 = reduction_row.find(t->tag);
		if (it2 != reduction_row.end()) {
			std::cout << "reducing via rule ";
			Parser::debug_production(it2->second);
			std::unique_ptr<MatchedNonterminal> mnt(new MatchedNonterminal(it2->second));
			for (size_t i = 0; i < it2->second->symbols.size(); i++) {
				this->parse_stack.pop();
				std::unique_ptr<Match> m = std::move(this->parse_stack_matches.top());
				this->parse_stack_matches.pop();
				//std::cout << "Removing entry ";
				//Parser::debug_match(m.get(), 0);
				mnt->terms[it2->second->symbols.size() - i - 1] = std::move(m);
			}
			mnt->value = it2->second->handler(mnt.get());
			this->parse_stack_matches.push(std::move(mnt));
			last_reduction = it2->second->target;
			if (t->tag != Parser::EPSILON) {
				mdk::printf("[debug] pushing token %s\n", t->tag.c_str());
				this->push_token(std::move(t));
			}
			continue;
		} else {
			// TODO: what is this?
			if (current_state->next.find(Parser::EPSILON) != current_state->next.end()) {
				mdk::printf("[debug] shift has epsilon\n");
				this->push_token(std::move(t));
				this->push_token(std::unique_ptr<Token>(new Token(Parser::EPSILON, "", LocationInfo(0, 0))));
			} else if (reduction_row.find(Parser::EPSILON) != reduction_row.end()) {
				mdk::printf("[debug] reduction has epsilon\n");
				this->push_token(std::move(t));
				this->push_token(std::unique_ptr<Token>(new Token(Parser::EPSILON, "", LocationInfo(0, 0))));
			} else {
				std::cout << "No reduction" << std::endl;
			}
		}
	}
	mdk::printf("[debug] parse stack size %zd\n", this->parse_stack.size());
	Parser::debug_match(this->parse_stack_matches.top().get(), 0);
	assert(this->parse_stack.size() == 1);
	assert(this->parse_stack_matches.size() == 1);
	std::unique_ptr<Match> ret = std::move(this->parse_stack_matches.top());
	this->parse_stack_matches.pop();
	return ret;
}

#pragma mark - Parser - private
void Parser::reset() {
	this->lexer.reset();
	while (!this->parse_stack.empty()) {
		this->parse_stack.pop();
	}
	while (!this->parse_stack_matches.empty()) {
		this->parse_stack_matches.pop();
	}
}

ItemSet* Parser::generate_itemset(std::set<Item> head) {
	std::cout << "=== Generating head:" << std::endl;
	for (const Item& x : head) {
		std::cout << "\t";
		Parser::debug_item(x);
	}
	std::cout << "=== done head ";
	std::map<std::set<Item>, ItemSet*>::iterator it = this->itemsets.find(head);
	if (it != this->itemsets.end()) {
		std::cout << "(exists)" << std::endl;
		return it->second;
	}
	std::cout << "(new)" << std::endl;
	std::unique_ptr<ItemSet> is(new ItemSet);
	is->head = head;
	is->index = this->states.size();
	ItemSet* ret = is.get();
	this->itemsets[head] = ret;
	this->states.push_back(std::move(is));
	std::set<std::string> encountered_terminals;
	for (const Item& item : head) {
		if (Parser::item_is_done(item)) {
			continue;
		}
		std::string next_symbol = item.first->symbols.at(item.second);
		this->expand_symbol_into_itemset(ret, next_symbol, &encountered_terminals);
	}

	std::set<Item> combined(ret->head);
	combined.insert(ret->additionals.begin(), ret->additionals.end());
	assert(combined.size() == ret->head.size() + ret->additionals.size());
	for (const Item& item : combined) {
		if (Parser::item_is_done(item)) {
			continue;
		}
		std::string next_symbol = item.first->symbols.at(item.second);
		if (ret->next.find(next_symbol) == ret->next.end()) {
			std::set<Item> h2;
			for (const Item& i2 : combined) {
				if (Parser::item_is_done(i2)) {
					continue;
				}
				std::string ns2 = i2.first->symbols.at(i2.second);
				if (ns2 == next_symbol) {
					h2.insert(Item(i2.first, i2.second + 1));
				}
			}
			ret->next[next_symbol] = this->generate_itemset(h2);
		}
	}
	return ret;
}

void Parser::expand_symbol_into_itemset(ItemSet* is, std::string symbol, std::set<std::string>* encountered_terminals) {
	if (Parser::symbol_is_token(symbol) || Parser::symbol_is_epsilon(symbol)) {
		std::cout << "=== expand symbol into itemset: token " << symbol << std::endl;
		return;
	} else {
		// if new symbol was inserted
		if (encountered_terminals->insert(symbol).second) {
			std::cout << "=== expand symbol into itemset: new nonterminal " << symbol << std::endl;
			for (Production* p : this->nonterminals.at(symbol)) {
				std::cout << "\t";
				Parser::debug_production(p);
				is->additionals.insert(Item(p, 0));
				this->expand_symbol_into_itemset(is, p->symbols.front(), encountered_terminals);
			}
		} else {
			std::cout << "=== expand symbol into itemset: existing nonterminal " << symbol << std::endl;
		}
	}
}

void Parser::generate_extended_grammar() {
	Int num = 0;
	auto fn = [&num](Parser* self, Item item, ItemSet* is) -> void {
		if (item.second != 0) {
			return;
		}
		std::cout << num << ". ";
		Parser::debug_item(item);
		Symbol target = item.first->target;
		std::unique_ptr<ExtendedProduction> ep(new ExtendedProduction);
		ep->target = ExtendedSymbol(target, is->index, (target == self->start) ? -1 : is->next.at(target)->index);
		ep->orig = item.first;
		Int i = 0;
		ItemSet* curr = is;
		for (std::string& s : item.first->symbols) {
			ItemSet* next = curr->next.at(s);
			ep->symbols.push_back(ExtendedSymbol(s, curr->index, next->index));
			curr = next;
			i++;
		}
		Parser::debug_extended_production(ep.get());
		self->extended_nonterminals[ep->target].push_back(ep.get());
		self->extended_grammar.push_back(std::move(ep));
		num++;
	};
	for (auto& kv : this->itemsets) {
		for (auto& item : kv.first) {
			fn(this, item, kv.second);
		}
		for (auto& item : kv.second->additionals) {
			fn(this, item, kv.second);
		}
	}
}

/*
 * Dragon book page 221
 */
void Parser::generate_extended_first_sets() {
	for (const std::unique_ptr<ExtendedProduction>& ep : this->extended_grammar) {
		for (ExtendedSymbol const& es : ep->symbols) {
			Symbol s = std::get<0>(es);
			if (Parser::symbol_is_token(s) || Parser::symbol_is_epsilon(s)) {
				this->extended_firsts[es].insert(s);
			}
		}
	}
	bool changed = true;
	while (changed) {
		changed = false;
		for (const std::unique_ptr<ExtendedProduction>& ep : this->extended_grammar) {
			mdk::printf("[debug] generating first set for\n");
			Parser::debug_extended_production(ep.get());
			if (Parser::production_is_epsilon(ep->orig)) {
				changed = changed || this->extended_firsts[ep->target].insert(Parser::EPSILON).second;
			} else {
				size_t old = this->extended_firsts[ep->target].size();
				// if `FIRST(ep->target)` contains `epsilon`, make sure we don't remove it later
				std::set<std::string>::iterator it = this->extended_firsts[ep->target].find(Parser::EPSILON);
				for (const ExtendedSymbol& es : ep->symbols) {
					/*
					Symbol s = std::get<0>(es);
					if (Parser::symbol_is_token(s)) {
						changed = changed || this->extended_firsts[es].insert(s).second;
						this->extended_firsts[ep->target].insert(s);
					} else {
					*/
						// TODO: there may be a bug here regarding nullable productions
						this->extended_firsts[ep->target].insert(this->extended_firsts[es].begin(), this->extended_firsts[es].end());
						//mdk::printf("[debug] inserting %s to %s is: ", s.c_str(), std::get<0>(ep->target).c_str());
						Parser::debug_set(this->extended_firsts[ep->target]);
						if (this->extended_firsts[es].find(Parser::EPSILON) == this->extended_firsts[es].end()) {
							if (it == this->extended_firsts[ep->target].end()) {
								this->extended_firsts[ep->target].erase(Parser::EPSILON);
							}
							break;
						}
					//}
				}
				changed = changed || (this->extended_firsts[ep->target].size() != old);
			}
		}
	}
}

void Parser::generate_extended_follow_sets() {
	bool changed = true;
	while (changed) {
		changed = false;
		for (auto& kv : this->extended_nonterminals) {
			std::cout << "\t- generating extended follow ";
			Parser::debug_extended_symbol(kv.first);
			std::cout << std::endl;
			if (std::get<2>(kv.first) == -1) {
				this->extended_follows[kv.first].insert(Parser::END);
			}
			for (ExtendedProduction* ep : kv.second) {
				if (Parser::production_is_epsilon(ep->orig)) {
					continue;
				}
				size_t len = ep->symbols.size();
				assert(len > 0);
				ExtendedSymbol z = ep->symbols.at(len - 1);
				if (!Parser::symbol_is_token(std::get<0>(z))) {
					size_t old = this->extended_follows[z].size();
					this->extended_follows[z].insert(this->extended_follows[ep->target].begin(), this->extended_follows[ep->target].end());
					changed = changed || (this->extended_follows[z].size() != old);
				}
				for (size_t i = 0; i < len - 1; i++) {
					ExtendedSymbol x = ep->symbols.at(i);
					if (!Parser::symbol_is_token(std::get<0>(x))) {
						ExtendedSymbol y = ep->symbols.at(i + 1);
						size_t old = this->extended_follows[x].size();
						std::cout << "\t\t- adding to ";
						Parser::debug_extended_symbol(x);
						std::cout << " from ";
						Parser::debug_extended_symbol(y);
						std::cout << std::endl;
						this->extended_follows[x].insert(this->extended_firsts[y].begin(), this->extended_firsts[y].end());
						changed = changed || (this->extended_follows[x].size() != old);
					}
				}
			}
		}
	}
}

void Parser::generate_reductions() {
	this->reductions.resize(this->itemsets.size());
	std::map<Int, Production*> lookup;
	for (size_t i = 0; i < this->extended_grammar.size(); i++) {
		std::unique_ptr<ExtendedProduction>& p1 = this->extended_grammar.at(i);
		Int final_set = std::get<2>(p1->symbols.back());

		std::map<Int, Production*>::iterator it = lookup.find(final_set);
		if (it == lookup.end()) {
			lookup[final_set] = p1->orig;
		} else {
			assert(it->second == p1->orig);
		}

		for (const Symbol& s : this->extended_follows.at(p1->target)) {
			std::map<Symbol, Production*>::iterator it = this->reductions.at(final_set).find(s);
			if (it == this->reductions.at(final_set).end()) {
				this->reductions.at(final_set)[s] = p1->orig;
				mdk::printf("[debug] creating reduction from %d to %s from %s\n", final_set, p1->orig->target.c_str(), s.c_str());
			} else {
				assert(it->second == p1->orig);
			}
		}

		// God knows what this is and why it's commented out
		/*
		for (size_t j = i + 1; j < this->extended_grammar.size(); j++) {
			std::unique_ptr<ExtendedProduction>& p2 = this->extended_grammar.at(j);
			if (p1->orig == p2->orig && final_set == std::get<2>(p2->symbols.back())) {
				for (const Symbol& s : this->extended_follows.at(p1->target)) {
					std::map<Symbol, Production*>::iterator it = this->reductions.at(final_set).find(s);
					if (it == this->reductions.at(final_set).end()) {
						this->reductions.at(final_set)[s] = p1->orig;
					} else {
						assert(it->second == p1->orig);
					}
				}
			}
		}
		*/
	}
	std::cout << "===== Reductions" << std::endl;
	Int i = 0;
	for (const std::map<Symbol, Production*>& col : this->reductions) {
		std::cout << "=== ItemSet " << i << std::endl;
		for (auto& kv : col) {
			std::cout << "- " << kv.first << ": ";
			Parser::debug_production(kv.second);
		}
		std::cout << std::endl;
		i++;
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

bool Parser::symbol_is_token(std::string str) {
	UInt ch = str[0];
	return ('A' <= ch) && (ch <= 'Z');
}

bool Parser::symbol_is_epsilon(std::string str) {
	return str == Parser::EPSILON;
}

bool Parser::production_is_epsilon(Production* p) {
	return p->symbols.size() == 1 && Parser::symbol_is_epsilon(p->symbols.front());
}

bool Parser::item_is_done(Item item) {
	return item.second == (Int) item.first->symbols.size();
}

#pragma mark - Parser - debug
void Parser::debug_production(Production* p, Int dot) {
	std::cout << p->target << " ::=";
	Int i = 0;
	for (auto& s : p->symbols) {
		if (i == dot) {
			std::cout << " .";
		}
		std::cout << " " << s;
		i++;
	}
	if (i == dot) {
		std::cout << " .";
	}
	std::cout << std::endl;
}

void Parser::debug_item(Item item) {
	Parser::debug_production(item.first, (Int) item.second);
}

void Parser::debug_extended_symbol(ExtendedSymbol es) {
	//std::cout << std::get<1>(es) << "_" << std::get<0>(es) << "_" << std::get<2>(es);
	std::cout << std::get<0>(es) << "(" << std::get<1>(es) << ", " << std::get<2>(es) << ")";
}

void Parser::debug_extended_production(ExtendedProduction* ep) {
	std::cout << "\t";
	Parser::debug_extended_symbol(ep->target);
	std::cout << " ::=";
	for (auto& es : ep->symbols) {
		std::cout << " ";
		Parser::debug_extended_symbol(es);
	}
	std::cout << std::endl;
}

void Parser::debug_set(std::set<std::string> s) {
	std::cout << "{";
	for (auto& str : s) {
		std::cout << " " << str;
	}
	std::cout << " }" << std::endl;
}

void Parser::debug_match(Match* m, Int levels) {
	std::string indent(levels, '\t');
	std::cout << indent;
	if (MatchedTerminal* mt = dynamic_cast<MatchedTerminal*>(m)) {
		std::cout << "{ matched terminal " + mt->token->tag << ": " << mt->token->lexeme << " }" << std::endl;
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

void Parser::debug(Parser* p) {
	std::cout << "===== Hmmmmm" << std::endl;
	for (auto& is : p->states) {
		std::cout << "=== Item Set " << is->index << std::endl;
		std::cout << "Head:" << std::endl;
		for (auto& x : is->head) {
			std::cout << "\t";
			Parser::debug_item(x);
		}
		std::cout << "Additionals:" << std::endl;
		for (auto& x : is->additionals) {
			std::cout << "\t";
			Parser::debug_item(x);
		}
		std::cout << "Next states:" << std::endl;
		for (auto& kv : is->next) {
			std::cout << "\t" << kv.first << " -> " << kv.second->index << std::endl;
		}
		std::cout << "=== done " << is->index << std::endl;
		std::cout << std::endl;
	}
	std::cout << "=== Extended firsts" << std::endl;
	for (auto& kv : p->extended_firsts) {
		std::cout << "\t";
		Parser::debug_extended_symbol(kv.first);
		std::cout << ": ";
		Parser::debug_set(kv.second);
	}
	std::cout << "=== Extended follows" << std::endl;
	for (auto& kv : p->extended_follows) {
		std::cout << "\t";
		Parser::debug_extended_symbol(kv.first);
		std::cout << ": ";
		Parser::debug_set(kv.second);
	}
}
