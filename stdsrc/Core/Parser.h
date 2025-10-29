#pragma once
#ifndef _Core_Parser_h_
#define _Core_Parser_h_

#include <string>
#include <vector>
#include <functional>
#include <cctype>
#include "Core.h"

// Simple character-based parser
class Parser {
private:
    const std::string& text;
    size_t pos;
    size_t line;
    size_t col;
    
public:
    explicit Parser(const std::string& input) : text(input), pos(0), line(1), col(1) {}
    
    // Check if at end of input
    bool IsEof() const { return pos >= text.length(); }
    
    // Get current character
    char Peek() const {
        if (IsEof()) return '\0';
        return text[pos];
    }
    
    // Consume current character
    char Take() {
        if (IsEof()) return '\0';
        
        char c = text[pos];
        pos++;
        
        if (c == '\n') {
            line++;
            col = 1;
        } else {
            col++;
        }
        
        return c;
    }
    
    // Consume current character if it matches expected
    bool TakeIf(char expected) {
        if (Peek() == expected) {
            Take();
            return true;
        }
        return false;
    }
    
    // Skip whitespace
    void SkipWhitespace() {
        while (!IsEof() && std::isspace(Peek())) {
            Take();
        }
    }
    
    // Skip while condition is true
    void SkipWhile(std::function<bool(char)> condition) {
        while (!IsEof() && condition(Peek())) {
            Take();
        }
    }
    
    // Parse identifier (alphanumeric + underscore, starting with letter)
    std::string ParseIdentifier() {
        std::string result;
        if (IsEof() || (!std::isalpha(Peek()) && Peek() != '_')) return result;
        
        result += Take();
        while (!IsEof() && (std::isalnum(Peek()) || Peek() == '_')) {
            result += Take();
        }
        
        return result;
    }
    
    // Parse number (integer or floating-point)
    std::string ParseNumber() {
        std::string result;
        if (IsEof() || (!std::isdigit(Peek()) && Peek() != '.')) return result;
        
        // Handle leading sign
        if (Peek() == '+' || Peek() == '-') {
            result += Take();
        }
        
        // Parse digits
        while (!IsEof() && std::isdigit(Peek())) {
            result += Take();
        }
        
        // Handle decimal point
        if (Peek() == '.') {
            result += Take();
            while (!IsEof() && std::isdigit(Peek())) {
                result += Take();
            }
        }
        
        return result;
    }
    
    // Parse string literal (with double quotes)
    std::string ParseString() {
        std::string result;
        if (IsEof() || Peek() != '"') return result;
        
        // Skip opening quote
        Take();
        
        while (!IsEof() && Peek() != '"') {
            char c = Take();
            if (c == '\\' && !IsEof()) {
                // Handle escape sequences
                c = Take();
                switch (c) {
                    case 'n': result += '\n'; break;
                    case 't': result += '\t'; break;
                    case 'r': result += '\r'; break;
                    case '\\': result += '\\'; break;
                    case '"': result += '"'; break;
                    default: result += c; break;
                }
            } else {
                result += c;
            }
        }
        
        // Skip closing quote
        if (!IsEof() && Peek() == '"') {
            Take();
        }
        
        return result;
    }
    
    // Parse token until one of the specified delimiters
    std::string ParseUntil(const std::string& delimiters) {
        std::string result;
        while (!IsEof() && delimiters.find(Peek()) == std::string::npos) {
            result += Take();
        }
        return result;
    }
    
    // Checkpoint and restore
    struct Position {
        size_t pos;
        size_t line;
        size_t col;
    };
    
    Position GetPosition() const {
        return {pos, line, col};
    }
    
    void SetPosition(const Position& p) {
        pos = p.pos;
        line = p.line;
        col = p.col;
    }
    
    // Get current line and column
    size_t GetLine() const { return line; }
    size_t GetColumn() const { return col; }
    size_t GetPositionIndex() const { return pos; }
    
    // Consume specific string
    bool Consume(const std::string& expected) {
        size_t saved_pos = pos;
        size_t saved_line = line;
        size_t saved_col = col;
        
        for (char c : expected) {
            if (IsEof() || Take() != c) {
                pos = saved_pos;
                line = saved_line;
                col = saved_col;
                return false;
            }
        }
        
        return true;
    }
};

#endif