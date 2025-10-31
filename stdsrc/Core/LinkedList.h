#pragma once
#ifndef _Core_LinkedList_h_
#define _Core_LinkedList_h_

#include <memory>
#include <iterator>
#include <algorithm>
#include "Core.h"

// Double-linked list node for stdsrc
template <class T>
struct DoublyLinked {
    DoublyLinked* next;
    DoublyLinked* prev;
    
    DoublyLinked() : next(nullptr), prev(nullptr) {}
    
    void Unlink() {
        if (prev) prev->next = next;
        if (next) next->prev = prev;
        next = prev = nullptr;
    }
    
    void InsertBefore(DoublyLinked* node) {
        if (!node) return;
        node->Unlink();
        node->next = this;
        node->prev = prev;
        if (prev) prev->next = node;
        prev = node;
    }
    
    void InsertAfter(DoublyLinked* node) {
        if (!node) return;
        node->Unlink();
        node->next = next;
        node->prev = this;
        if (next) next->prev = node;
        next = node;
    }
    
    bool IsLinked() const {
        return next || prev;
    }
};

// Linked list implementation for stdsrc
template <class T>
class LinkedList {
private:
    struct Node : DoublyLinked<T> {
        T data;
        
        template<typename... Args>
        Node(Args&&... args) : data(std::forward<Args>(args)...) {}
    };
    
    Node* head;
    Node* tail;
    size_t count;
    
public:
    // Constructors
    LinkedList() : head(nullptr), tail(nullptr), count(0) {}
    
    LinkedList(const LinkedList& other) : head(nullptr), tail(nullptr), count(0) {
        for (const auto& item : other) {
            AddTail(item);
        }
    }
    
    LinkedList(LinkedList&& other) noexcept 
        : head(other.head), tail(other.tail), count(other.count) {
        other.head = nullptr;
        other.tail = nullptr;
        other.count = 0;
    }
    
    LinkedList& operator=(const LinkedList& other) {
        if (this != &other) {
            Clear();
            for (const auto& item : other) {
                AddTail(item);
            }
        }
        return *this;
    }
    
    LinkedList& operator=(LinkedList&& other) noexcept {
        if (this != &other) {
            Clear();
            head = other.head;
            tail = other.tail;
            count = other.count;
            other.head = nullptr;
            other.tail = nullptr;
            other.count = 0;
        }
        return *this;
    }
    
    // Destructor
    ~LinkedList() {
        Clear();
    }
    
    // Element access
    T& Get(int index) {
        Node* node = GetNode(index);
        if (!node) {
            throw std::out_of_range("Index out of range");
        }
        return node->data;
    }
    
    const T& Get(int index) const {
        const Node* node = GetNode(index);
        if (!node) {
            throw std::out_of_range("Index out of range");
        }
        return node->data;
    }
    
    T& operator[](int index) {
        return Get(index);
    }
    
    const T& operator[](int index) const {
        return Get(index);
    }
    
    T& First() {
        if (!head) {
            throw std::runtime_error("List is empty");
        }
        return head->data;
    }
    
    const T& First() const {
        if (!head) {
            throw std::runtime_error("List is empty");
        }
        return head->data;
    }
    
    T& Last() {
        if (!tail) {
            throw std::runtime_error("List is empty");
        }
        return tail->data;
    }
    
    const T& Last() const {
        if (!tail) {
            throw std::runtime_error("List is empty");
        }
        return tail->data;
    }
    
    // Adding elements
    template<typename... Args>
    T& AddHead(Args&&... args) {
        std::unique_ptr<Node> newNode = std::make_unique<Node>(std::forward<Args>(args)...);
        Node* node = newNode.get();
        
        if (head) {
            head->prev = node;
            node->next = head;
        } else {
            tail = node;
        }
        
        head = node;
        items.push_back(std::move(newNode));
        count++;
        return node->data;
    }
    
    template<typename... Args>
    T& AddTail(Args&&... args) {
        std::unique_ptr<Node> newNode = std::make_unique<Node>(std::forward<Args>(args)...);
        Node* node = newNode.get();
        
        if (tail) {
            tail->next = node;
            node->prev = tail;
        } else {
            head = node;
        }
        
        tail = node;
        items.push_back(std::move(newNode));
        count++;
        return node->data;
    }
    
    template<typename... Args>
    T& Insert(int index, Args&&... args) {
        if (index < 0 || index > static_cast<int>(count)) {
            throw std::out_of_range("Index out of range");
        }
        
        if (index == 0) {
            return AddHead(std::forward<Args>(args)...);
        }
        
        if (index == static_cast<int>(count)) {
            return AddTail(std::forward<Args>(args)...);
        }
        
        Node* after = GetNode(index);
        std::unique_ptr<Node> newNode = std::make_unique<Node>(std::forward<Args>(args)...);
        Node* node = newNode.get();
        
        node->next = after;
        node->prev = after->prev;
        if (after->prev) {
            after->prev->next = node;
        }
        after->prev = node;
        
        if (after == head) {
            head = node;
        }
        
        items.push_back(std::move(newNode));
        count++;
        return node->data;
    }
    
    // Removing elements
    void Remove(int index) {
        Node* node = GetNode(index);
        if (!node) {
            throw std::out_of_range("Index out of range");
        }
        
        if (node->prev) {
            node->prev->next = node->next;
        } else {
            head = node->next;
        }
        
        if (node->next) {
            node->next->prev = node->prev;
        } else {
            tail = node->prev;
        }
        
        // Find and remove from items vector
        auto it = std::find_if(items.begin(), items.end(), 
                              [node](const std::unique_ptr<Node>& ptr) { return ptr.get() == node; });
        if (it != items.end()) {
            items.erase(it);
        }
        
        count--;
    }
    
    void RemoveHead() {
        if (!head) {
            throw std::runtime_error("List is empty");
        }
        Remove(0);
    }
    
    void RemoveTail() {
        if (!tail) {
            throw std::runtime_error("List is empty");
        }
        Remove(GetCount() - 1);
    }
    
    // Clear all elements
    void Clear() {
        head = nullptr;
        tail = nullptr;
        items.clear();
        count = 0;
    }
    
    // Size
    size_t GetCount() const {
        return count;
    }
    
    bool IsEmpty() const {
        return count == 0;
    }
    
    // Iterators
    class Iterator {
    private:
        Node* node;
        
    public:
        Iterator(Node* n) : node(n) {}
        
        T& operator*() { return node->data; }
        T* operator->() { return &node->data; }
        
        Iterator& operator++() { node = static_cast<Node*>(node->next); return *this; }
        Iterator& operator--() { node = static_cast<Node*>(node->prev); return *this; }
        
        bool operator==(const Iterator& other) const { return node == other.node; }
        bool operator!=(const Iterator& other) const { return node != other.node; }
        
        Node* GetNode() const { return node; }
    };
    
    class ConstIterator {
    private:
        const Node* node;
        
    public:
        ConstIterator(const Node* n) : node(n) {}
        
        const T& operator*() const { return node->data; }
        const T* operator->() const { return &node->data; }
        
        ConstIterator& operator++() { node = static_cast<const Node*>(node->next); return *this; }
        ConstIterator& operator--() { node = static_cast<const Node*>(node->prev); return *this; }
        
        bool operator==(const ConstIterator& other) const { return node == other.node; }
        bool operator!=(const ConstIterator& other) const { return node != other.node; }
        
        const Node* GetNode() const { return node; }
    };
    
    Iterator begin() { return Iterator(head); }
    Iterator end() { return Iterator(nullptr); }
    
    ConstIterator begin() const { return ConstIterator(head); }
    ConstIterator end() const { return ConstIterator(nullptr); }
    
    ConstIterator cbegin() const { return ConstIterator(head); }
    ConstIterator cend() const { return ConstIterator(nullptr); }
    
    // Reverse iterators
    class ReverseIterator {
    private:
        Node* node;
        
    public:
        ReverseIterator(Node* n) : node(n) {}
        
        T& operator*() { return node->data; }
        T* operator->() { return &node->data; }
        
        ReverseIterator& operator++() { node = static_cast<Node*>(node->prev); return *this; }
        ReverseIterator& operator--() { node = static_cast<Node*>(node->next); return *this; }
        
        bool operator==(const ReverseIterator& other) const { return node == other.node; }
        bool operator!=(const ReverseIterator& other) const { return node != other.node; }
        
        Node* GetNode() const { return node; }
    };
    
    class ConstReverseIterator {
    private:
        const Node* node;
        
    public:
        ConstReverseIterator(const Node* n) : node(n) {}
        
        const T& operator*() const { return node->data; }
        const T* operator->() const { return &node->data; }
        
        ConstReverseIterator& operator++() { node = static_cast<const Node*>(node->prev); return *this; }
        ConstReverseIterator& operator--() { node = static_cast<const Node*>(node->next); return *this; }
        
        bool operator==(const ConstReverseIterator& other) const { return node == other.node; }
        bool operator!=(const ConstReverseIterator& other) const { return node != other.node; }
        
        const Node* GetNode() const { return node; }
    };
    
    ReverseIterator rbegin() { return ReverseIterator(tail); }
    ReverseIterator rend() { return ReverseIterator(nullptr); }
    
    ConstReverseIterator rbegin() const { return ConstReverseIterator(tail); }
    ConstReverseIterator rend() const { return ConstReverseIterator(nullptr); }
    
    ConstReverseIterator crbegin() const { return ConstReverseIterator(tail); }
    ConstReverseIterator crend() const { return ConstReverseIterator(nullptr); }
    
    // Find operations
    template<typename Predicate>
    int FindIndex(Predicate pred) const {
        int index = 0;
        for (const Node* node = head; node; node = static_cast<const Node*>(node->next), ++index) {
            if (pred(node->data)) {
                return index;
            }
        }
        return -1;
    }
    
    int Find(const T& item) const {
        return FindIndex([&item](const T& data) { return data == item; });
    }
    
    bool Contains(const T& item) const {
        return Find(item) >= 0;
    }
    
    // Swap
    void Swap(LinkedList& other) {
        std::swap(head, other.head);
        std::swap(tail, other.tail);
        std::swap(count, other.count);
        items.swap(other.items);
    }
    
    // Serialization support
    template<typename Stream>
    void Serialize(Stream& s) {
        int n = static_cast<int>(count);
        s / n;
        
        if (s.IsLoading()) {
            Clear();
            for (int i = 0; i < n; ++i) {
                T item;
                s % item;
                AddTail(std::move(item));
            }
        } else {
            for (Node* node = head; node; node = static_cast<Node*>(node->next)) {
                s % node->data;
            }
        }
    }
    
    // String representation
    std::string ToString() const {
        std::ostringstream oss;
        oss << "[";
        bool first = true;
        for (const auto& item : *this) {
            if (!first) oss << ", ";
            oss << item;
            first = false;
        }
        oss << "]";
        return oss.str();
    }
    
private:
    std::vector<std::unique_ptr<Node>> items;
    
    Node* GetNode(int index) const {
        if (index < 0 || index >= static_cast<int>(count)) {
            return nullptr;
        }
        
        Node* node = head;
        for (int i = 0; i < index && node; ++i) {
            node = static_cast<Node*>(node->next);
        }
        return node;
    }
};

// Global swap function
template<class T>
void Swap(LinkedList<T>& a, LinkedList<T>& b) {
    a.Swap(b);
}

// Streaming operator
template<typename Stream, class T>
void operator%(Stream& s, LinkedList<T>& list) {
    list.Serialize(s);
}

// String conversion
template<class T>
std::string AsString(const LinkedList<T>& list) {
    return list.ToString();
}

#endif