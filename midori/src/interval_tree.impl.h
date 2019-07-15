#ifndef MIDORI_INTERVAL_TREE_IMPL_H_INCLUDED
#define MIDORI_INTERVAL_TREE_IMPL_H_INCLUDED

#include "interval_tree.h"

#pragma mark - IntervalTree

template <typename K, typename V> void IntervalTree<K, V>::insert(Interval key, V value) {
	this->root = Node::insert(std::move(this->root), key, value);
}

// see: https://stackoverflow.com/questions/610245/where-and-why-do-i-have-to-put-the-template-and-typename-keywords
template <typename K, typename V> std::unique_ptr<typename IntervalTree<K, V>::SearchList> IntervalTree<K, V>::pop(Interval key) {
	std::unique_ptr<SearchList> results(new SearchList);
	while (true) {
		std::unique_ptr<Node> n;
		this->root = Node::pop(std::move(this->root), key, &n);
		if (n == nullptr) {
			break;
		}
		results->push_back(SearchResult(n->key, n->value));
	}
	return results;
}

template <typename K, typename V> std::unique_ptr<typename IntervalTree<K, V>::SearchList> IntervalTree<K, V>::all() {
	std::unique_ptr<SearchList> results(new SearchList);
	if (this->root != nullptr) {
		Node::all(this->root.get(), results.get());
	}
	return results;
}

template <typename K, typename V> void IntervalTree<K, V>::invariants() {
	Node::invariants(this->root.get());
}

#pragma mark - IntervalTree::Node

template <typename K, typename V> Int IntervalTree<K, V>::Node::_height(Node* n) {
	if (n == nullptr) {
		return 0;
	}
	return n->height;
}

template <typename K, typename V> std::unique_ptr<typename IntervalTree<K, V>::Node> IntervalTree<K, V>::Node::balance(std::unique_ptr<Node> self) {
	Int factor = self->balance_factor();
	if (factor * factor <= 1) {
		return self;
	}
	Int sub_factor = (factor < 0) ? self->left->balance_factor() : self->right->balance_factor();
	if (factor * sub_factor > 0) {
		return Node::rotate(std::move(self), factor);
	}
	if (factor == 2) {
		self->right = Node::rotate(std::move(self->right), sub_factor);
	} else {
		self->left = Node::rotate(std::move(self->left), sub_factor);
	}
	return Node::rotate(std::move(self), factor);
}

template <typename K, typename V> std::unique_ptr<typename IntervalTree<K, V>::Node> IntervalTree<K, V>::Node::rotate(std::unique_ptr<Node> self, Int factor) {
	if (factor < 0) {
		return Node::rotate_cw(std::move(self));
	} else if (factor > 0) {
		return Node::rotate_ccw(std::move(self));
	}
	return self;
}

template <typename K, typename V> std::unique_ptr<typename IntervalTree<K, V>::Node> IntervalTree<K, V>::Node::rotate_cw(std::unique_ptr<Node> self) {
	std::unique_ptr<Node> n(std::move(self->left));
	self->left.reset(n->right.release());
	n->right.reset(self.release());
	n->right->update();
	n->update();
	return n;
}

template <typename K, typename V> std::unique_ptr<typename IntervalTree<K, V>::Node> IntervalTree<K, V>::Node::rotate_ccw(std::unique_ptr<Node> self) {
	std::unique_ptr<Node> n(std::move(self->right));
	self->right.reset(n->left.release());
	n->left.reset(self.release());
	n->left->update();
	n->update();
	return n;
}

template <typename K, typename V> std::unique_ptr<typename IntervalTree<K, V>::Node> IntervalTree<K, V>::Node::insert(std::unique_ptr<Node> self, Interval key, V value) {
	if (self == nullptr) {
		std::unique_ptr<Node> n(new Node);
		n->key = key;
		n->value = value;
		n->update();
		return n;
	}
	if (key < self->key) {
		self->left = Node::insert(std::move(self->left), key, value);
	} else if (key > self->key) {
		self->right = Node::insert(std::move(self->right), key, value);
	} else {
		self->value = value;
	}
	self->update();
	return Node::balance(std::move(self));
}

template <typename K, typename V> std::unique_ptr<typename IntervalTree<K, V>::Node> IntervalTree<K, V>::Node::pop_min(std::unique_ptr<Node> self, std::unique_ptr<Node>* ret) {
	assert(self != nullptr);
	if (self->left == nullptr) {
		std::unique_ptr<Node> n(std::move(self->right));
		ret->reset(self.release());
		return n;
	}
	self->left = Node::pop_min(std::move(self->left), ret);
	self->update();
	return Node::balance(std::move(self));
}

template <typename K, typename V> std::unique_ptr<typename IntervalTree<K, V>::Node> IntervalTree<K, V>::Node::pop(std::unique_ptr<Node> self, Interval key, std::unique_ptr<Node>* ret) {
	if (self == nullptr) {
		return nullptr;
	}
	if (!self->overlaps_recursive(key)) {
		return self;
	}
	self->left = Node::pop(std::move(self->left), key, ret);
	if (ret->get() != nullptr) {
		self->update();
		return Node::balance(std::move(self));
	}
	if (self->overlaps(key)) {
		Node* n = self.get();
		ret->reset(self.release());
		if ((n->left == nullptr) && (n->right == nullptr)) {
			return nullptr;
		} else if (n->left == nullptr) {
			return std::move(n->right);
		} else if (n->right == nullptr) {
			return std::move(n->left);
		} else {
			std::unique_ptr<Node> replacement;
			std::unique_ptr<Node> subtree = Node::pop_min(std::move(n->right), &replacement);
			assert(replacement->left == nullptr);
			replacement->left = std::move(n->left);
			assert(replacement->right == nullptr);
			replacement->right = std::move(subtree);
			replacement->update();
			return Node::balance(std::move(replacement));
		}
	}
	self->right = Node::pop(std::move(self->right), key, ret);
	if (ret->get() != nullptr) {
		self->update();
		return Node::balance(std::move(self));
	}
	return self;
}

template <typename K, typename V> void IntervalTree<K, V>::Node::all(Node* self, SearchList* results) {
	if (self == nullptr) {
		return;
	}
	Node::all(self->left.get(), results);
	results->push_back(SearchResult(self->key, self->value));
	Node::all(self->right.get(), results);
}

template <typename K, typename V> Int IntervalTree<K, V>::Node::balance_factor() {
	return Node::_height(this->right.get()) - Node::_height(this->left.get());
}

template <typename K, typename V> void IntervalTree<K, V>::Node::update() {
	if ((this->left == nullptr) && (this->right == nullptr)) {
		this->height = 1;
		this->lower = this->key.first;
		this->upper = this->key.second;
	} else if (this->left == nullptr) {
		this->height = 2;
		this->lower = this->key.first;
		this->upper = std::max(this->key.second, this->right->upper);
	} else if (this->right == nullptr) {
		this->height = 2;
		this->lower = this->left->lower;
		this->upper = std::max(this->key.second, this->left->upper);
	} else {
		this->height = std::max(this->left->height, this->right->height) + 1;
		this->lower = this->left->lower;
		this->upper = std::max({ this->key.second, this->left->upper, this->right->upper });
	}
}

template <typename K, typename V> bool IntervalTree<K, V>::Node::contains(K point) {
	return this->key.first <= point && point <= this->key.second;
}

template <typename K, typename V> bool IntervalTree<K, V>::Node::contains_recursive(K point) {
	return this->lower <= point && point <= this->upper;
}

template <typename K, typename V> bool IntervalTree<K, V>::Node::overlaps(Interval key) {
	return this->contains(key.first) || this->contains(key.second) || ((key.first <= this->key.first) && (this->key.first <= key.second));
}

template <typename K, typename V> bool IntervalTree<K, V>::Node::overlaps_recursive(Interval key) {
	return this->contains_recursive(key.first) || this->contains_recursive(key.second) || ((key.first <= this->lower) && (this->lower <= key.second));
}

#pragma mark - IntervalTree::Node - debug

template <typename K, typename V> void IntervalTree<K, V>::Node::invariants(Node* self) {
	if (self == nullptr) {
		return;
	}
	Node::invariants(self->left.get());
	Node::invariants(self->right.get());
	Int x = self->balance_factor();
	assert(x * x <= 1);
	assert(self->height == std::max(Node::_height(self->left.get()), Node::_height(self->right.get())) + 1);
	if (self->left == nullptr) {
		assert(self->lower == self->key.first);
		if (self->right == nullptr) {
			assert(self->upper == self->key.second);
		} else {
			assert(self->upper == std::max(self->key.second, self->right->upper));
		}
	} else {
		assert(self->lower == self->left->lower);
		if (self->right == nullptr) {
			assert(self->upper == std::max(self->key.second, self->left->upper));
		} else {
			assert(self->upper == std::max({ self->key.second, self->left->upper, self->right->upper }));
		}
	}
}

#endif /* MIDORI_INTERVAL_TREE_IMPL_H_INCLUDED */
