#pragma once
#ifndef _Core_LinkedList_h_
#define _Core_LinkedList_h_

#include <memory>
#include <functional>
#include "Core.h"

// Simple linked list node
template <typename T>
struct ListNode {
    T data;
    std::unique_ptr<ListNode<T>> next;
    
    ListNode() = default;
    ListNode(const T& value) : data(value), next(nullptr) {}
    ListNode(T&& value) : data(std::move(value)), next(nullptr) {}
    
    // Note: Using default move constructor/assignment since unique_ptr is not copyable
    ListNode(const ListNode&) = delete;
    ListNode& operator=(const ListNode&) = delete;
};

// Singly linked list implementation
template <typename T>
class LinkedList {
private:
    std::unique_ptr<ListNode<T>> head;
    size_t count;

public:
    LinkedList() : head(nullptr), count(0) {}
    
    // Move constructor/assignment
    LinkedList(LinkedList&& other) noexcept 
        : head(std::move(other.head)), count(other.count) {
        other.count = 0;
    }
    
    LinkedList& operator=(LinkedList&& other) noexcept {
        if (this != &other) {
            head = std::move(other.head);
            count = other.count;
            other.count = 0;
        }
        return *this;
    }
    
    // Not copyable
    LinkedList(const LinkedList&) = delete;
    LinkedList& operator=(const LinkedList&) = delete;
    
    ~LinkedList() = default;
    
    // Add element to front
    void PushFront(const T& value) {
        auto node = std::make_unique<ListNode<T>>(value);
        node->next = std::move(head);
        head = std::move(node);
        count++;
    }
    
    void PushFront(T&& value) {
        auto node = std::make_unique<ListNode<T>>(std::move(value));
        node->next = std::move(head);
        head = std::move(node);
        count++;
    }
    
    // Add element to back
    void PushBack(const T& value) {
        if (!head) {
            PushFront(value);
            return;
        }
        
        ListNode<T>* current = head.get();
        while (current->next) {
            current = current->next.get();
        }
        
        current->next = std::make_unique<ListNode<T>>(value);
        count++;
    }
    
    void PushBack(T&& value) {
        if (!head) {
            PushFront(std::move(value));
            return;
        }
        
        ListNode<T>* current = head.get();
        while (current->next) {
            current = current->next.get();
        }
        
        current->next = std::make_unique<ListNode<T>>(std::move(value));
        count++;
    }
    
    // Remove element from front
    bool PopFront() {
        if (!head) return false;
        
        head = std::move(head->next);
        count--;
        return true;
    }
    
    // Get front element reference
    T& Front() {
        return head->data;
    }
    
    const T& Front() const {
        return head->data;
    }
    
    // Check if empty
    bool IsEmpty() const {
        return head == nullptr;
    }
    
    // Get count
    size_t GetCount() const {
        return count;
    }
    
    // Clear all elements
    void Clear() {
        head.reset();
        count = 0;
    }
    
    // Find element
    bool Contains(const T& value) const {
        ListNode<T>* current = head.get();
        while (current) {
            if (current->data == value) {
                return true;
            }
            current = current->next.get();
        }
        return false;
    }
    
    // ForEach operation
    void ForEach(std::function<void(const T&)> fn) const {
        ListNode<T>* current = head.get();
        while (current) {
            fn(current->data);
            current = current->next.get();
        }
    }
    
    void ForEach(std::function<void(T&)> fn) {
        ListNode<T>* current = head.get();
        while (current) {
            fn(current->data);
            current = current->next.get();
        }
    }
    
    // Find with predicate
    template<typename Predicate>
    ListNode<T>* FindIf(Predicate pred) {
        ListNode<T>* current = head.get();
        while (current) {
            if (pred(current->data)) {
                return current;
            }
            current = current->next.get();
        }
        return nullptr;
    }
    
    // Remove element matching predicate
    template<typename Predicate>
    bool RemoveIf(Predicate pred) {
        if (!head) return false;
        
        if (pred(head->data)) {
            head = std::move(head->next);
            count--;
            return true;
        }
        
        ListNode<T>* current = head.get();
        while (current->next) {
            if (pred(current->next->data)) {
                current->next = std::move(current->next->next);
                count--;
                return true;
            }
            current = current->next.get();
        }
        return false;
    }
};

#endif