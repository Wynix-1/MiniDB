#pragma once

#include "string.hpp"
#include "array.hpp"

namespace mini_db {

template<typename K, typename V>
class BPlusTree {
private:
    static constexpr int ORDER = 4;

    struct Node {
        bool is_leaf;
        Array<K> keys;
        Array<V> values;
        Array<Node*> children;
        Node* next;

        Node(bool leaf = true) : is_leaf(leaf), next(nullptr) {}

        ~Node() {
            for (size_t i = 0; i < children.size(); ++i) {
                delete children[i];
            }
        }
    };

    Node* root_;

    Node* create_node(bool is_leaf) {
        return new Node(is_leaf);
    }

    void insert_non_full(Node* node, const K& key, const V& value) {
        size_t i = node->keys.size();

        if (node->is_leaf) {
            node->keys.resize(node->keys.size() + 1);
            node->values.resize(node->values.size() + 1);
            while (i > 0 && key < node->keys[i - 1]) {
                node->keys[i] = node->keys[i - 1];
                node->values[i] = node->values[i - 1];
                --i;
            }
            node->keys[i] = key;
            node->values[i] = value;
        } else {
            while (i > 0 && key < node->keys[i - 1]) {
                --i;
            }

            if (node->children[i]->keys.size() == ORDER - 1) {
                split_child(node, static_cast<int>(i));
                if (key > node->keys[i]) {
                    ++i;
                }
            }
            insert_non_full(node->children[i], key, value);
        }
    }

    void split_child(Node* parent, int index) {
        Node* full_child = parent->children[index];
        Node* new_node = create_node(full_child->is_leaf);

        K mid_key = full_child->keys[ORDER / 2 - 1];

        for (int i = ORDER / 2; i < ORDER - 1; ++i) {
            new_node->keys.push_back(full_child->keys[i]);
            if (!full_child->is_leaf) {
                new_node->children.push_back(full_child->children[i]);
            }
        }
        if (!full_child->is_leaf) {
            new_node->children.push_back(full_child->children[ORDER - 1]);
        }

        if (full_child->is_leaf) {
            for (int i = ORDER / 2 - 1; i < ORDER - 1; ++i) {
                new_node->values.push_back(full_child->values[i]);
            }
            new_node->next = full_child->next;
            full_child->next = new_node;
        }

        full_child->keys.resize(ORDER / 2 - 1);
        if (full_child->is_leaf) {
            full_child->values.resize(ORDER / 2 - 1);
        }
        if (!full_child->is_leaf) {
            full_child->children.resize(ORDER / 2);
        }

        parent->keys.insert(parent->keys.begin() + index, mid_key);
        parent->children.insert(parent->children.begin() + index + 1, new_node);
    }

    V* search(Node* node, const K& key) const {
        size_t i = 0;
        while (i < node->keys.size() && key > node->keys[i]) {
            ++i;
        }

        if (i < node->keys.size() && key == node->keys[i]) {
            if (node->is_leaf) {
                return &(node->values[i]);
            }
            return search(node->children[i], key);
        }

        if (node->is_leaf) {
            return nullptr;
        }

        return search(node->children[i], key);
    }

    void remove(Node* node, const K& key) {
        if (!node || node->keys.empty()) return;

        size_t i = 0;
        while (i < node->keys.size() && key > node->keys[i]) {
            ++i;
        }

        if (node->is_leaf) {
            if (i < node->keys.size() && key == node->keys[i]) {
                node->keys.erase(node->keys.begin() + i);
                node->values.erase(node->values.begin() + i);
            }
        } else {
            if (i < node->keys.size() && key == node->keys[i]) {
                Node* pred = node->children[i];
                while (!pred->is_leaf) {
                    pred = pred->children[pred->children.size() - 1];
                }
                K pred_key = pred->keys[pred->keys.size() - 1];
                node->keys[i] = pred_key;
                remove(node->children[i], pred_key);
            } else {
                remove(node->children[i], key);
            }
        }
    }

    void destroy_tree(Node* node) {
        if (!node) return;
        if (!node->is_leaf) {
            for (size_t i = 0; i < node->children.size(); ++i) {
                destroy_tree(node->children[i]);
            }
        }
        delete node;
    }

    Node* copy_tree(Node* node) {
        if (!node) return nullptr;
        Node* new_node = create_node(node->is_leaf);
        new_node->keys = node->keys;
        new_node->values = node->values;
        if (!node->is_leaf) {
            for (size_t i = 0; i < node->children.size(); ++i) {
                new_node->children.push_back(copy_tree(node->children[i]));
            }
        }
        return new_node;
    }

public:
    BPlusTree() : root_(nullptr) {}

    BPlusTree(const BPlusTree& other) : root_(copy_tree(other.root_)) {}

    BPlusTree(BPlusTree&& other) noexcept : root_(other.root_) {
        other.root_ = nullptr;
    }

    BPlusTree& operator=(const BPlusTree& other) {
        if (this != &other) {
            destroy_tree(root_);
            root_ = copy_tree(other.root_);
        }
        return *this;
    }

    BPlusTree& operator=(BPlusTree&& other) noexcept {
        if (this != &other) {
            destroy_tree(root_);
            root_ = other.root_;
            other.root_ = nullptr;
        }
        return *this;
    }

    ~BPlusTree() {
        destroy_tree(root_);
    }

    void insert(const K& key, const V& value) {
        if (!root_) {
            root_ = create_node(true);
            root_->keys.push_back(key);
            root_->values.push_back(value);
            return;
        }

        if (root_->keys.size() == ORDER - 1) {
            Node* new_root = create_node(false);
            new_root->children.push_back(root_);
            split_child(new_root, 0);
            root_ = new_root;
        }

        insert_non_full(root_, key, value);
    }

    V* search(const K& key) const {
        return root_ ? search(root_, key) : nullptr;
    }

    void remove(const K& key) {
        if (root_) {
            remove(root_, key);
        }
    }

    bool empty() const { return !root_ || root_->keys.empty(); }

    Array<V> range_search(const K& start_key, const K& end_key) {
        Array<V> results;
        if (!root_) return results;

        Node* node = root_;
        while (!node->is_leaf) {
            size_t i = 0;
            while (i < node->keys.size() && start_key > node->keys[i]) {
                ++i;
            }
            node = node->children[i];
        }

        size_t i = 0;
        while (i < node->keys.size() && node->keys[i] < start_key) {
            ++i;
        }

        while (node) {
            while (i < node->keys.size() && node->keys[i] <= end_key) {
                results.push_back(node->values[i]);
                ++i;
            }
            if (i >= node->keys.size() || node->keys[i] > end_key) {
                node = node->next;
                i = 0;
            }
        }

        return results;
    }
};

} // namespace mini_db
