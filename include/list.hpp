#pragma once

#include <cstddef>
#include <utility>

namespace mini_db {

template<typename T>
class List {
public:
    using value_type = T;
    using size_type = std::size_t;
    using reference = T&;
    using const_reference = const T&;

private:
    struct Node {
        T data;
        Node* prev;
        Node* next;

        Node(const T& val) : data(val), prev(nullptr), next(nullptr) {}
        Node(T&& val) : data(std::move(val)), prev(nullptr), next(nullptr) {}
    };

public:
    class Iterator {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        Iterator(Node* node) : node_(node) {}

        reference operator*() const { return node_->data; }
        pointer operator->() { return &(node_->data); }

        Iterator& operator++() { node_ = node_->next; return *this; }
        Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }
        Iterator& operator--() { node_ = node_->prev; return *this; }
        Iterator operator--(int) { Iterator tmp = *this; --(*this); return tmp; }

        bool operator==(const Iterator& other) const { return node_ == other.node_; }
        bool operator!=(const Iterator& other) const { return node_ != other.node_; }

    private:
        Node* node_;
    };

    class ConstIterator {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = const T*;
        using reference = const T&;

        ConstIterator(Node* node) : node_(node) {}

        reference operator*() const { return node_->data; }
        pointer operator->() { return &(node_->data); }

        ConstIterator& operator++() { node_ = node_->next; return *this; }
        ConstIterator operator++(int) { ConstIterator tmp = *this; ++(*this); return tmp; }
        ConstIterator& operator--() { node_ = node_->prev; return *this; }
        ConstIterator operator--(int) { ConstIterator tmp = *this; --(*this); return tmp; }

        bool operator==(const ConstIterator& other) const { return node_ == other.node_; }
        bool operator!=(const ConstIterator& other) const { return node_ != other.node_; }

    private:
        Node* node_;
    };

    List() : head_(nullptr), tail_(nullptr), size_(0) {}

    List(std::initializer_list<T> init) : head_(nullptr), tail_(nullptr), size_(0) {
        for (const auto& item : init) {
            push_back(item);
        }
    }

    List(const List& other) : head_(nullptr), tail_(nullptr), size_(0) {
        Node* current = other.head_;
        while (current) {
            push_back(current->data);
            current = current->next;
        }
    }

    List(List&& other) noexcept : head_(other.head_), tail_(other.tail_), size_(other.size_) {
        other.head_ = nullptr;
        other.tail_ = nullptr;
        other.size_ = 0;
    }

    List& operator=(const List& other) {
        if (this != &other) {
            clear();
            Node* current = other.head_;
            while (current) {
                push_back(current->data);
                current = current->next;
            }
        }
        return *this;
    }

    List& operator=(List&& other) noexcept {
        if (this != &other) {
            clear();
            head_ = other.head_;
            tail_ = other.tail_;
            size_ = other.size_;
            other.head_ = nullptr;
            other.tail_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }

    ~List() {
        clear();
    }

    bool empty() const { return size_ == 0; }
    size_type size() const { return size_; }

    reference front() { return head_->data; }
    const_reference front() const { return head_->data; }

    reference back() { return tail_->data; }
    const_reference back() const { return tail_->data; }

    void push_front(const T& value) {
        Node* new_node = new Node(value);
        if (!head_) {
            head_ = tail_ = new_node;
        } else {
            new_node->next = head_;
            head_->prev = new_node;
            head_ = new_node;
        }
        ++size_;
    }

    void push_front(T&& value) {
        Node* new_node = new Node(std::move(value));
        if (!head_) {
            head_ = tail_ = new_node;
        } else {
            new_node->next = head_;
            head_->prev = new_node;
            head_ = new_node;
        }
        ++size_;
    }

    void push_back(const T& value) {
        Node* new_node = new Node(value);
        if (!tail_) {
            head_ = tail_ = new_node;
        } else {
            new_node->prev = tail_;
            tail_->next = new_node;
            tail_ = new_node;
        }
        ++size_;
    }

    void push_back(T&& value) {
        Node* new_node = new Node(std::move(value));
        if (!tail_) {
            head_ = tail_ = new_node;
        } else {
            new_node->prev = tail_;
            tail_->next = new_node;
            tail_ = new_node;
        }
        ++size_;
    }

    void pop_front() {
        if (!head_) return;
        Node* temp = head_;
        head_ = head_->next;
        if (head_) {
            head_->prev = nullptr;
        } else {
            tail_ = nullptr;
        }
        delete temp;
        --size_;
    }

    void pop_back() {
        if (!tail_) return;
        Node* temp = tail_;
        tail_ = tail_->prev;
        if (tail_) {
            tail_->next = nullptr;
        } else {
            head_ = nullptr;
        }
        delete temp;
        --size_;
    }

    void clear() {
        while (head_) {
            Node* temp = head_;
            head_ = head_->next;
            delete temp;
        }
        tail_ = nullptr;
        size_ = 0;
    }

    Iterator begin() { return Iterator(head_); }
    Iterator end() { return Iterator(nullptr); }
    ConstIterator begin() const { return ConstIterator(head_); }
    ConstIterator end() const { return ConstIterator(nullptr); }
    ConstIterator cbegin() const { return ConstIterator(head_); }
    ConstIterator cend() const { return ConstIterator(nullptr); }

    Iterator insert(Iterator pos, const T& value) {
        Node* current = pos.node_;
        if (!current) {
            push_back(value);
            return Iterator(tail_);
        }
        if (current == head_) {
            push_front(value);
            return Iterator(head_);
        }
        Node* new_node = new Node(value);
        new_node->prev = current->prev;
        new_node->next = current;
        current->prev->next = new_node;
        current->prev = new_node;
        ++size_;
        return Iterator(new_node);
    }

    Iterator erase(Iterator pos) {
        Node* current = pos.node_;
        if (!current) return end();
        if (current == head_) {
            pop_front();
            return Iterator(head_);
        }
        if (current == tail_) {
            pop_back();
            return Iterator(nullptr);
        }
        current->prev->next = current->next;
        current->next->prev = current->prev;
        Node* next = current->next;
        delete current;
        --size_;
        return Iterator(next);
    }

    bool operator==(const List& other) const {
        if (size_ != other.size_) return false;
        Node* this_node = head_;
        Node* other_node = other.head_;
        while (this_node && other_node) {
            if (this_node->data != other_node->data) return false;
            this_node = this_node->next;
            other_node = other_node->next;
        }
        return true;
    }

    bool operator!=(const List& other) const {
        return !(*this == other);
    }

private:
    Node* head_;
    Node* tail_;
    size_type size_;
};

} // namespace mini_db
