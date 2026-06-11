#pragma once

#include <cstddef>
#include <initializer_list>
#include <stdexcept>
#include <new>
#include <memory>
#include <algorithm>
#include <utility>

namespace mini_db {

template<typename T>
class Array {
public:
    using value_type = T;
    using size_type = std::size_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;

    class Iterator {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        Iterator(pointer ptr) : ptr_(ptr) {}

        reference operator*() const { return *ptr_; }
        pointer operator->() { return ptr_; }

        Iterator& operator++() { ++ptr_; return *this; }
        Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }
        Iterator& operator--() { --ptr_; return *this; }
        Iterator operator--(int) { Iterator tmp = *this; --(*this); return tmp; }

        Iterator& operator+=(difference_type n) { ptr_ += n; return *this; }
        Iterator& operator-=(difference_type n) { ptr_ -= n; return *this; }

        Iterator operator+(difference_type n) const { return Iterator(ptr_ + n); }
        Iterator operator-(difference_type n) const { return Iterator(ptr_ - n); }
        difference_type operator-(const Iterator& other) const { return ptr_ - other.ptr_; }

        reference operator[](difference_type n) const { return ptr_[n]; }

        bool operator==(const Iterator& other) const { return ptr_ == other.ptr_; }
        bool operator!=(const Iterator& other) const { return ptr_ != other.ptr_; }
        bool operator<(const Iterator& other) const { return ptr_ < other.ptr_; }
        bool operator>(const Iterator& other) const { return ptr_ > other.ptr_; }
        bool operator<=(const Iterator& other) const { return ptr_ <= other.ptr_; }
        bool operator>=(const Iterator& other) const { return ptr_ >= other.ptr_; }

    private:
        pointer ptr_;
    };

    class ConstIterator {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = const T*;
        using reference = const T&;

        ConstIterator(pointer ptr) : ptr_(ptr) {}

        reference operator*() const { return *ptr_; }
        pointer operator->() { return ptr_; }

        ConstIterator& operator++() { ++ptr_; return *this; }
        ConstIterator operator++(int) { ConstIterator tmp = *this; ++(*this); return tmp; }
        ConstIterator& operator--() { --ptr_; return *this; }
        ConstIterator operator--(int) { ConstIterator tmp = *this; --(*this); return tmp; }

        ConstIterator& operator+=(difference_type n) { ptr_ += n; return *this; }
        ConstIterator& operator-=(difference_type n) { ptr_ -= n; return *this; }

        ConstIterator operator+(difference_type n) const { return ConstIterator(ptr_ + n); }
        ConstIterator operator-(difference_type n) const { return ConstIterator(ptr_ - n); }
        difference_type operator-(const ConstIterator& other) const { return ptr_ - other.ptr_; }

        reference operator[](difference_type n) const { return ptr_[n]; }

        bool operator==(const ConstIterator& other) const { return ptr_ == other.ptr_; }
        bool operator!=(const ConstIterator& other) const { return ptr_ != other.ptr_; }
        bool operator<(const ConstIterator& other) const { return ptr_ < other.ptr_; }
        bool operator>(const ConstIterator& other) const { return ptr_ > other.ptr_; }
        bool operator<=(const ConstIterator& other) const { return ptr_ <= other.ptr_; }
        bool operator>=(const ConstIterator& other) const { return ptr_ >= other.ptr_; }

    private:
        pointer ptr_;
    };

    Array() : data_(nullptr), size_(0), capacity_(0) {}

    explicit Array(size_type count) : data_(nullptr), size_(count), capacity_(count) {
        if (count > 0) {
            data_ = static_cast<pointer>(::operator new(count * sizeof(T)));
            for (size_type i = 0; i < count; ++i) {
                new (data_ + i) T();
            }
        }
    }

    Array(size_type count, const T& value) : data_(nullptr), size_(count), capacity_(count) {
        if (count > 0) {
            data_ = static_cast<pointer>(::operator new(count * sizeof(T)));
            for (size_type i = 0; i < count; ++i) {
                new (data_ + i) T(value);
            }
        }
    }

    Array(std::initializer_list<T> init) : data_(nullptr), size_(init.size()), capacity_(init.size()) {
        if (capacity_ > 0) {
            data_ = static_cast<pointer>(::operator new(capacity_ * sizeof(T)));
            size_type i = 0;
            for (const auto& item : init) {
                new (data_ + i) T(item);
                ++i;
            }
        }
    }

    Array(const Array& other) : data_(nullptr), size_(other.size_), capacity_(other.capacity_) {
        if (capacity_ > 0) {
            data_ = static_cast<pointer>(::operator new(capacity_ * sizeof(T)));
            for (size_type i = 0; i < size_; ++i) {
                new (data_ + i) T(other.data_[i]);
            }
        }
    }

    Array(Array&& other) noexcept : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    Array& operator=(const Array& other) {
        if (this != &other) {
            clear();
            ::operator delete(data_);
            data_ = nullptr;
            size_ = 0;
            capacity_ = 0;

            size_ = other.size_;
            capacity_ = other.capacity_;
            if (capacity_ > 0) {
                data_ = static_cast<pointer>(::operator new(capacity_ * sizeof(T)));
                for (size_type i = 0; i < size_; ++i) {
                    new (data_ + i) T(other.data_[i]);
                }
            }
        }
        return *this;
    }

    Array& operator=(Array&& other) noexcept {
        if (this != &other) {
            clear();
            ::operator delete(data_);
            data_ = other.data_;
            size_ = other.size_;
            capacity_ = other.capacity_;
            other.data_ = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
        }
        return *this;
    }

    ~Array() {
        clear();
        ::operator delete(data_);
    }

    reference at(size_type pos) {
        if (pos >= size_) {
            throw std::out_of_range("Array index out of range");
        }
        return data_[pos];
    }

    const_reference at(size_type pos) const {
        if (pos >= size_) {
            throw std::out_of_range("Array index out of range");
        }
        return data_[pos];
    }

    reference operator[](size_type pos) { return data_[pos]; }
    const_reference operator[](size_type pos) const { return data_[pos]; }

    reference front() { return data_[0]; }
    const_reference front() const { return data_[0]; }

    reference back() { return data_[size_ - 1]; }
    const_reference back() const { return data_[size_ - 1]; }

    pointer data() noexcept { return data_; }
    const_pointer data() const noexcept { return data_; }

    bool empty() const { return size_ == 0; }
    size_type size() const { return size_; }
    size_type capacity() const { return capacity_; }

    void reserve(size_type new_cap) {
        if (new_cap <= capacity_) return;

        pointer new_data = static_cast<pointer>(::operator new(new_cap * sizeof(T)));
        for (size_type i = 0; i < size_; ++i) {
            new (new_data + i) T(std::move(data_[i]));
            data_[i].~T();
        }
        ::operator delete(data_);
        data_ = new_data;
        capacity_ = new_cap;
    }

    void push_back(const T& value) {
        if (size_ == capacity_) {
            size_type new_cap = capacity_ == 0 ? 1 : capacity_ * 2;
            reserve(new_cap);
        }
        new (data_ + size_) T(value);
        ++size_;
    }

    void push_back(T&& value) {
        if (size_ == capacity_) {
            size_type new_cap = capacity_ == 0 ? 1 : capacity_ * 2;
            reserve(new_cap);
        }
        new (data_ + size_) T(std::move(value));
        ++size_;
    }

    void pop_back() {
        if (size_ > 0) {
            --size_;
            data_[size_].~T();
        }
    }

    void clear() {
        for (size_type i = 0; i < size_; ++i) {
            data_[i].~T();
        }
        size_ = 0;
    }

    Iterator begin() { return Iterator(data_); }
    Iterator end() { return Iterator(data_ + size_); }
    ConstIterator begin() const { return ConstIterator(data_); }
    ConstIterator end() const { return ConstIterator(data_ + size_); }
    ConstIterator cbegin() const { return ConstIterator(data_); }
    ConstIterator cend() const { return ConstIterator(data_ + size_); }

    Iterator insert(Iterator pos, const T& value) {
        size_type index = pos - begin();
        if (size_ == capacity_) {
            size_type new_cap = capacity_ == 0 ? 1 : capacity_ * 2;
            reserve(new_cap);
        }
        for (size_type i = size_; i > index; --i) {
            new (data_ + i) T(std::move(data_[i - 1]));
            data_[i - 1].~T();
        }
        new (data_ + index) T(value);
        ++size_;
        return Iterator(data_ + index);
    }

    Iterator erase(Iterator pos) {
        size_type index = pos - begin();
        for (size_type i = index; i < size_ - 1; ++i) {
            data_[i] = std::move(data_[i + 1]);
        }
        --size_;
        data_[size_].~T();
        return Iterator(data_ + index);
    }

    void resize(size_type count) {
        if (count > size_) {
            if (count > capacity_) {
                reserve(count);
            }
            for (size_type i = size_; i < count; ++i) {
                new (data_ + i) T();
            }
        } else if (count < size_) {
            for (size_type i = count; i < size_; ++i) {
                data_[i].~T();
            }
        }
        size_ = count;
    }

    void resize(size_type count, const T& value) {
        if (count > size_) {
            if (count > capacity_) {
                reserve(count);
            }
            for (size_type i = size_; i < count; ++i) {
                new (data_ + i) T(value);
            }
        } else if (count < size_) {
            for (size_type i = count; i < size_; ++i) {
                data_[i].~T();
            }
        }
        size_ = count;
    }

    bool operator==(const Array& other) const {
        if (size_ != other.size_) return false;
        for (size_type i = 0; i < size_; ++i) {
            if (data_[i] != other.data_[i]) return false;
        }
        return true;
    }

    bool operator!=(const Array& other) const {
        return !(*this == other);
    }

private:
    pointer data_;
    size_type size_;
    size_type capacity_;
};

} // namespace mini_db
