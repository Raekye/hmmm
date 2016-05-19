#include "parser.h"
#include <cassert>

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
	this->nonterminals[target].push_back(p.get());
	this->productions.push_back(std::move(p));
}

void Parser::parse(std::istream* in) {
	(void) in;
	this->generate();
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
	// generate all the items
	// advance dot
	// generate all the next items
	// call generate on those items
	std::set<Item> combined;
	combined.insert(ret->head.begin(), ret->head.end());
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
				Parser::debug_production(p);
				is->additionals.insert(Item(p, 0));
				this->expand_symbol_into_itemset(is, p->symbols.front(), encountered_terminals);
			}
		} else {
			std::cout << "=== expand symbol into itemset: existing nonterminal " << symbol << std::endl;
		}
	}
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

void Parser::debug_production(Production* p, Int dot) {
	std::cout << "\t" << p->target << " ::=";
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

void Parser::debug(Parser* p) {
	std::cout << "===== Hmmmmm" << std::endl;
	for (auto& is : p->states) {
		std::cout << "=== Item Set " << is->index << std::endl;
		std::cout << "Head:" << std::endl;
		for (auto& x : is->head) {
			Parser::debug_item(x);
		}
		std::cout << "Additionals:" << std::endl;
		for (auto& x : is->additionals) {
			Parser::debug_item(x);
		}
		std::cout << "Next states:" << std::endl;
		for (auto& kv : is->next) {
			std::cout << "\t" << kv.first << " -> " << kv.second->index << std::endl;
		}
		std::cout << "=== done " << is->index << std::endl;
		std::cout << std::endl;
	}
}
