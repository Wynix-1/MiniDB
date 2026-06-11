#pragma once

#include <cstring>
#include <algorithm>
#include <stdexcept>

namespace mini_db {

class String {
public:
    using size_type = std::size_t;
    static constexpr size_type npos = static_cast<size_type>(-1);

    String() : data_(nullptr), size_(0), capacity_(0) {}

    String(const char* str) {
        if (str) {
            size_ = std::strlen(str);
            capacity_ = size_ + 1;
            data_ = new char[capacity_];
            std::memcpy(data_, str, size_ + 1);
        } else {
            data_ = nullptr;
            size_ = 0;
            capacity_ = 0;
        }
    }

    String(const char* str, size_type count) {
        size_ = count;
        capacity_ = count + 1;
        data_ = new char[capacity_];
        std::memcpy(data_, str, count);
        data_[count] = '\0';
    }

    String(size_type count, char ch) {
        size_ = count;
        capacity_ = count + 1;
        data_ = new char[capacity_];
        std::memset(data_, ch, count);
        data_[count] = '\0';
    }

    String(const String& other) : data_(nullptr), size_(other.size_), capacity_(other.capacity_) {
        if (capacity_ > 0) {
            data_ = new char[capacity_];
            std::memcpy(data_, other.data_, size_ + 1);
        }
    }

    String(String&& other) noexcept : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    String& operator=(const String& other) {
        if (this != &other) {
            delete[] data_;
            size_ = other.size_;
            capacity_ = other.capacity_;
            if (capacity_ > 0) {
                data_ = new char[capacity_];
                std::memcpy(data_, other.data_, size_ + 1);
            } else {
                data_ = nullptr;
            }
        }
        return *this;
    }

    String& operator=(String&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            data_ = other.data_;
            size_ = other.size_;
            capacity_ = other.capacity_;
            other.data_ = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
        }
        return *this;
    }

    String& operator=(const char* str) {
        delete[] data_;
        if (str) {
            size_ = std::strlen(str);
            capacity_ = size_ + 1;
            data_ = new char[capacity_];
            std::memcpy(data_, str, size_ + 1);
        } else {
            data_ = nullptr;
            size_ = 0;
            capacity_ = 0;
        }
        return *this;
    }

    ~String() {
        delete[] data_;
    }

    const char* c_str() const noexcept { return data_ ? data_ : ""; }
    const char* data() const noexcept { return data_ ? data_ : ""; }

    size_type size() const noexcept { return size_; }
    size_type length() const noexcept { return size_; }
    bool empty() const noexcept { return size_ == 0; }
    size_type capacity() const noexcept { return capacity_; }

    char& operator[](size_type pos) { return data_[pos]; }
    const char& operator[](size_type pos) const { return data_[pos]; }

    char& at(size_type pos) {
        if (pos >= size_) throw std::out_of_range("String index out of range");
        return data_[pos];
    }

    const char& at(size_type pos) const {
        if (pos >= size_) throw std::out_of_range("String index out of range");
        return data_[pos];
    }

    void reserve(size_type new_cap) {
        if (new_cap <= capacity_) return;
        char* new_data = new char[new_cap];
        if (data_) {
            std::memcpy(new_data, data_, size_ + 1);
        }
        delete[] data_;
        data_ = new_data;
        capacity_ = new_cap;
    }

    void push_back(char ch) {
        if (size_ + 1 >= capacity_) {
            reserve(capacity_ == 0 ? 16 : capacity_ * 2);
        }
        data_[size_] = ch;
        ++size_;
        data_[size_] = '\0';
    }

    void append(const String& str) {
        append(str.data_, str.size_);
    }

    void append(const char* str) {
        append(str, std::strlen(str));
    }

    void append(const char* str, size_type count) {
        if (size_ + count + 1 > capacity_) {
            reserve((size_ + count + 1) * 2);
        }
        std::memcpy(data_ + size_, str, count);
        size_ += count;
        data_[size_] = '\0';
    }

    void append(size_type count, char ch) {
        if (size_ + count + 1 > capacity_) {
            reserve((size_ + count + 1) * 2);
        }
        std::memset(data_ + size_, ch, count);
        size_ += count;
        data_[size_] = '\0';
    }

    String& operator+=(const String& str) {
        append(str);
        return *this;
    }

    String& operator+=(const char* str) {
        append(str);
        return *this;
    }

    String& operator+=(char ch) {
        push_back(ch);
        return *this;
    }

    String substr(size_type pos = 0, size_type count = npos) const {
        if (pos > size_) throw std::out_of_range("String index out of range");
        size_type len = (count == npos || pos + count > size_) ? size_ - pos : count;
        return String(data_ + pos, len);
    }

    size_type find(const String& str, size_type pos = 0) const noexcept {
        return find(str.data_, pos);
    }

    size_type find(const char* str, size_type pos = 0) const noexcept {
        if (!str || pos >= size_) return npos;
        size_type len = std::strlen(str);
        if (len == 0) return pos;
        if (len > size_ - pos) return npos;
        const char* found = std::strstr(data_ + pos, str);
        return found ? static_cast<size_type>(found - data_) : npos;
    }

    size_type find(char ch, size_type pos = 0) const noexcept {
        if (pos >= size_) return npos;
        for (size_type i = pos; i < size_; ++i) {
            if (data_[i] == ch) return i;
        }
        return npos;
    }

    int compare(const String& str) const noexcept {
        return std::strcmp(data_ ? data_ : "", str.data_ ? str.data_ : "");
    }

    int compare(const char* str) const noexcept {
        return std::strcmp(data_ ? data_ : "", str ? str : "");
    }

    void clear() {
        if (data_) {
            data_[0] = '\0';
            size_ = 0;
        }
    }

    void pop_back() {
        if (size_ > 0) {
            --size_;
            data_[size_] = '\0';
        }
    }

    void trim() {
        if (size_ == 0) return;
        size_type start = 0;
        while (start < size_ && (data_[start] == ' ' || data_[start] == '\t' || data_[start] == '\n' || data_[start] == '\r')) {
            ++start;
        }
        size_type end = size_;
        while (end > start && (data_[end - 1] == ' ' || data_[end - 1] == '\t' || data_[end - 1] == '\n' || data_[end - 1] == '\r')) {
            --end;
        }
        if (start > 0 || end < size_) {
            size_type new_size = end - start;
            if (start > 0) {
                std::memmove(data_, data_ + start, new_size);
            }
            size_ = new_size;
            data_[size_] = '\0';
        }
    }

    String to_lower() const {
        String result(*this);
        for (size_type i = 0; i < result.size_; ++i) {
            if (result.data_[i] >= 'A' && result.data_[i] <= 'Z') {
                result.data_[i] = result.data_[i] - 'A' + 'a';
            }
        }
        return result;
    }

    String to_upper() const {
        String result(*this);
        for (size_type i = 0; i < result.size_; ++i) {
            if (result.data_[i] >= 'a' && result.data_[i] <= 'z') {
                result.data_[i] = result.data_[i] - 'a' + 'A';
            }
        }
        return result;
    }

    bool starts_with(const String& prefix) const noexcept {
        return starts_with(prefix.data_);
    }

    bool starts_with(const char* prefix) const noexcept {
        if (!prefix) return true;
        size_type len = std::strlen(prefix);
        if (len > size_) return false;
        return std::strncmp(data_, prefix, len) == 0;
    }

    bool ends_with(const String& suffix) const noexcept {
        return ends_with(suffix.data_);
    }

    bool ends_with(const char* suffix) const noexcept {
        if (!suffix) return true;
        size_type len = std::strlen(suffix);
        if (len > size_) return false;
        return std::strcmp(data_ + size_ - len, suffix) == 0;
    }

    friend String operator+(const String& lhs, const String& rhs);
    friend String operator+(const String& lhs, const char* rhs);
    friend String operator+(const char* lhs, const String& rhs);
    friend String operator+(const String& lhs, char rhs);

    friend bool operator==(const String& lhs, const String& rhs);
    friend bool operator==(const String& lhs, const char* rhs);
    friend bool operator==(const char* lhs, const String& rhs);
    friend bool operator!=(const String& lhs, const String& rhs);
    friend bool operator<(const String& lhs, const String& rhs);
    friend bool operator>(const String& lhs, const String& rhs);
    friend bool operator<=(const String& lhs, const String& rhs);
    friend bool operator>=(const String& lhs, const String& rhs);

private:
    char* data_;
    size_type size_;
    size_type capacity_;
};

inline String operator+(const String& lhs, const String& rhs) {
    String result(lhs);
    result.append(rhs);
    return result;
}

inline String operator+(const String& lhs, const char* rhs) {
    String result(lhs);
    result.append(rhs);
    return result;
}

inline String operator+(const char* lhs, const String& rhs) {
    String result(lhs);
    result.append(rhs);
    return result;
}

inline String operator+(const String& lhs, char rhs) {
    String result(lhs);
    result.push_back(rhs);
    return result;
}

inline bool operator==(const String& lhs, const String& rhs) {
    return lhs.compare(rhs) == 0;
}

inline bool operator==(const String& lhs, const char* rhs) {
    return lhs.compare(rhs) == 0;
}

inline bool operator==(const char* lhs, const String& rhs) {
    return rhs.compare(lhs) == 0;
}

inline bool operator!=(const String& lhs, const String& rhs) {
    return !(lhs == rhs);
}

inline bool operator<(const String& lhs, const String& rhs) {
    return lhs.compare(rhs) < 0;
}

inline bool operator>(const String& lhs, const String& rhs) {
    return lhs.compare(rhs) > 0;
}

inline bool operator<=(const String& lhs, const String& rhs) {
    return lhs.compare(rhs) <= 0;
}

inline bool operator>=(const String& lhs, const String& rhs) {
    return lhs.compare(rhs) >= 0;
}

} // namespace mini_db
