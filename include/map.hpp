#pragma once

#include "string.hpp"
#include "array.hpp"
#include <functional>

namespace mini_db {

template<typename K, typename V>
class Map {
private:
    struct Node {
        K key;
        V value;
        Node* left;
        Node* right;

        Node(const K& k, const V& v) : key(k), value(v), left(nullptr), right(nullptr) {}
    };

    Node* root_;
    size_t size_;

    void clear_tree(Node* node) {
        if (!node) return;
        clear_tree(node->left);
        clear_tree(node->right);
        delete node;
    }

    Node* copy_tree(Node* node) {
        if (!node) return nullptr;
        Node* new_node = new Node(node->key, node->value);
        new_node->left = copy_tree(node->left);
        new_node->right = copy_tree(node->right);
        return new_node;
    }

    Node* insert(Node* node, const K& key, const V& value) {
        if (!node) {
            ++size_;
            return new Node(key, value);
        }
        if (key < node->key) {
            node->left = insert(node->left, key, value);
        } else if (key > node->key) {
            node->right = insert(node->right, key, value);
        } else {
            node->value = value;
        }
        return node;
    }

    Node* find_min(Node* node) {
        while (node && node->left) {
            node = node->left;
        }
        return node;
    }

    Node* remove(Node* node, const K& key) {
        if (!node) return nullptr;
        if (key < node->key) {
            node->left = remove(node->left, key);
        } else if (key > node->key) {
            node->right = remove(node->right, key);
        } else {
            if (!node->left) {
                Node* temp = node->right;
                delete node;
                --size_;
                return temp;
            }
            if (!node->right) {
                Node* temp = node->left;
                delete node;
                --size_;
                return temp;
            }
            Node* min_right = find_min(node->right);
            node->key = min_right->key;
            node->value = min_right->value;
            node->right = remove(node->right, min_right->key);
        }
        return node;
    }

    Node* find(Node* node, const K& key) const {
        if (!node || key == node->key) return node;
        if (key < node->key) return find(node->left, key);
        return find(node->right, key);
    }

public:
    Map() : root_(nullptr), size_(0) {}

    Map(const Map& other) : root_(copy_tree(other.root_)), size_(other.size_) {}

    Map(Map&& other) noexcept : root_(other.root_), size_(other.size_) {
        other.root_ = nullptr;
        other.size_ = 0;
    }

    Map& operator=(const Map& other) {
        if (this != &other) {
            clear_tree(root_);
            root_ = copy_tree(other.root_);
            size_ = other.size_;
        }
        return *this;
    }

    Map& operator=(Map&& other) noexcept {
        if (this != &other) {
            clear_tree(root_);
            root_ = other.root_;
            size_ = other.size_;
            other.root_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }

    ~Map() {
        clear_tree(root_);
    }

    void insert(const K& key, const V& value) {
        root_ = insert(root_, key, value);
    }

    void remove(const K& key) {
        root_ = remove(root_, key);
    }

    V* find(const K& key) {
        Node* node = find(root_, key);
        return node ? &(node->value) : nullptr;
    }

    const V* find(const K& key) const {
        Node* node = find(root_, key);
        return node ? &(node->value) : nullptr;
    }

    bool contains(const K& key) const {
        return find(root_, key) != nullptr;
    }

    V& operator[](const K& key) {
        Node* node = find(root_, key);
        if (!node) {
            root_ = insert(root_, key, V());
            node = find(root_, key);
        }
        return node->value;
    }

    bool empty() const { return size_ == 0; }
    size_t size() const { return size_; }

    void clear() {
        clear_tree(root_);
        root_ = nullptr;
        size_ = 0;
    }

    Array<K> keys() const {
        Array<K> result;
        collect_keys(root_, result);
        return result;
    }

private:
    void collect_keys(Node* node, Array<K>& result) const {
        if (!node) return;
        collect_keys(node->left, result);
        result.push_back(node->key);
        collect_keys(node->right, result);
    }
};

} // namespace mini_db
