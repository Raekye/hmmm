#include "parser.h"
#include <cassert>

const std::string Parser::END = "$";
const std::string Parser::EPSILON = "NIL";

void Parser::set_start(std::string start) {
	this->start = start;
}

void Parser::add_token(std::string tag, std::string pattern) {
	this->lexer.add_rule(Rule(tag, pattern));
	this->terminals.insert(tag);
}

void Parser::add_production(std::string target, std::vector<std::string> symbols, ProductionHandler handler) {
	std::unique_ptr<Production> p(new Production);
	p->target = target;
	p->symbols = symbols;
	p->handler = handler;
	p->nullable = false;
	p->empty = true;
	for (std::string s : symbols) {
		if (Parser::symbol_is_epsilon(s)) {
			p->nullable = true;
		} else {
			p->empty = false;
		}
	}
	this->nonterminals[target].push_back(p.get());
	this->productions.push_back(std::move(p));
}

void Parser::parse(std::istream* in) {
	(void) in;

	this->generate();
	std::cout << std::endl;

	std::cout << "=== Generating extended grammar" << std::endl;
	this->generate_extended_grammar();
	std::cout << "=== Done extended grammar" << std::endl;
	std::cout << std::endl;

	std::cout << "=== Generating first and follow sets" << std::endl;
	this->generate_first_and_follow();
	std::cout << "=== Done first and follow sets" << std::endl;
	std::cout << std::endl;

	std::cout << "=== Generating extended sets" << std::endl;
	this->generate_extended_first_and_follow();
	std::cout << "=== Done extended sets" << std::endl;
	std::cout << std::endl;

	Parser::debug(this);
	return;
}

#pragma mark - Parser - private
void Parser::generate() {
	this->generate(this->start);
}

void Parser::generate(std::string symbol) {
	std::set<Item> head;
	for (auto p : this->nonterminals.at(symbol)) {
		head.insert(Item(p, 0));
	}
	this->generate_itemset(head);
}

ItemSet* Parser::generate_itemset(std::set<Item> head) {
	std::cout << "=== Generating head:" << std::endl;
	for (auto& x : head) {
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
	for (auto& item : head) {
		if (Parser::is_item_done(item)) {
			continue;
		}
		std::string next_symbol = item.first->symbols.at(item.second);
		this->expand_symbol_into_itemset(ret, next_symbol, &encountered_terminals);
	}

	std::set<Item> combined(ret->head);
	combined.insert(ret->additionals.begin(), ret->additionals.end());
	assert(combined.size() == ret->head.size() + ret->additionals.size());
	for (auto& item : combined) {
		if (Parser::is_item_done(item)) {
			continue;
		}
		std::string next_symbol = item.first->symbols.at(item.second);
		if (ret->next.find(next_symbol) == ret->next.end()) {
			std::set<Item> h2;
			for (auto& i2 : combined) {
				if (Parser::is_item_done(i2)) {
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
	if (Parser::symbol_is_token(symbol)) {
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

void Parser::generate_first_set(std::string symbol) {
	if (Parser::symbol_is_token(symbol)) {
		this->firsts[symbol].insert(symbol);
	} else {
		for (Production* p : this->nonterminals.at(symbol)) {
			if (p->empty) {
				this->firsts[symbol].insert(Parser::EPSILON);
			} else {
				size_t i = 0;
				for (std::string s : p->symbols) {
					this->generate_first_set(s);
					this->firsts[symbol].insert(this->firsts[s].begin(), this->firsts[s].end());
					if (!Parser::symbol_is_nullable(s)) {
						break;
					}
					i++;
				}
				if (i != p->symbols.size()) {
					this->firsts[symbol].erase(Parser::EPSILON);
				}
			}
		}
	}
}

void Parser::generate_extended_first_set(ExtendedSymbol es) {
	Symbol s = std::get<0>(es);
	if (Parser::symbol_is_token(s)) {
		this->extended_firsts[es].insert(s);
	} else {
		for (ExtendedProduction* ep : this->extended_nonterminals.at(es)) {
			if (ep->orig->empty) {
				this->extended_firsts[es].insert(Parser::EPSILON);
			} else {
				size_t i = 0;
				for (ExtendedSymbol& x : ep->symbols) {
					this->generate_extended_first_set(x);
					this->extended_firsts[es].insert(this->extended_firsts[x].begin(), this->extended_firsts[x].end());
					if (!Parser::symbol_is_nullable(std::get<0>(x))) {
						break;
					}
					i++;
				}
				if (i != ep->symbols.size()) {
					this->extended_firsts[es].erase(Parser::EPSILON);
				}
			}
		}
	}
}

void Parser::generate_follow_sets() {
	this->follows[this->start].insert(Parser::END);
	bool changed = true;
	while (changed) {
		changed = false;
		for (auto& kv : this->nonterminals) {
			for (Production* p : kv.second) {
				size_t len = p->symbols.size();
				assert(len > 0);
				std::string z = p->symbols.at(len - 1);
				if (!Parser::symbol_is_token(z)) {
					size_t old = this->follows[z].size();
					this->follows[z].insert(this->follows[p->target].begin(), this->follows[p->target].end());
					changed = changed || (this->follows[z].size() != old);
				}
				for (size_t i = 0; i < len - 1; i++) {
					std::string x = p->symbols.at(i);
					if (!Parser::symbol_is_token(x)) {
						std::string y = p->symbols.at(i + 1);
						size_t old = this->follows[x].size();
						this->follows[x].insert(this->firsts[y].begin(), this->firsts[y].end());
						changed = changed || (this->follows[x].size() != old);
					}
				}
			}
		}
	}
}

void Parser::generate_extended_follow_sets() {
	bool changed = true;
	while (changed) {
		changed = false;
		std::cout << "Heresauce" << std::endl;
		for (auto& kv : this->extended_nonterminals) {
			std::cout << "\t- generating extended follow ";
			Parser::debug_extended_symbol(kv.first);
			std::cout << std::endl;
			if (std::get<2>(kv.first) == -1) {
				this->extended_follows[kv.first].insert(Parser::END);
			}
			for (ExtendedProduction* ep : kv.second) {
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

void Parser::generate_extended_grammar() {
	Int num = 0;
	auto fn = [&num](Item item, ItemSet* is, Parser* self) -> void {
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
			fn(item, kv.second, this);
		}
		for (auto& item : kv.second->additionals) {
			fn(item, kv.second, this);
		}
	}
}

void Parser::generate_first_and_follow() {
	for (std::string s : this->terminals) {
		this->generate_first_set(s);
	}
	for (auto& kv : this->nonterminals) {
		this->generate_first_set(kv.first);
	}
	this->generate_follow_sets();
}

void Parser::generate_extended_first_and_follow() {
	for (auto& ep : this->extended_grammar) {
		this->generate_extended_first_set(ep->target);
		for (ExtendedSymbol& es : ep->symbols) {
			this->generate_extended_first_set(es);
		}
	}
	this->generate_extended_follow_sets();
}

bool Parser::is_item_done(Item item) {
	return item.second == item.first->symbols.size();
}

void Parser::push_token(Token* t) {
	this->token_buffer.push(t);
}

Token* Parser::next_token(std::istream* in) {
	if (this->token_buffer.empty()) {
		return this->lexer.scan(in);
	}
	Token* t = this->token_buffer.top();
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

bool Parser::symbol_is_nullable(std::string str) {
	if (Parser::symbol_is_token(str)) {
		return Parser::symbol_is_epsilon(str);
	}
	for (Production* p : this->nonterminals.at(str)) {
		if (p->nullable) {
			return true;
		}
	}
	return false;
}

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
	std::cout << "=== First sets" << std::endl;
	for (auto& kv : p->firsts) {
		std::cout << "\t" << kv.first << ": ";
		Parser::debug_set(kv.second);
	}
	std::cout << "=== Follow sets" << std::endl;
	for (auto& kv : p->follows) {
		std::cout << "\t" << kv.first << ": ";
		Parser::debug_set(kv.second);
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
