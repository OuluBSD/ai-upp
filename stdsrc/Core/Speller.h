#pragma once
#ifndef _Core_Speller_h_
#define _Core_Speller_h_

#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <cctype>
#include "Core.h"

// Speller - spell checking and correction utilities for stdsrc

class Speller {
private:
    std::set<std::string> dictionary;
    std::map<std::string, std::vector<std::string>> corrections;
    int max_edit_distance;
    
    // Helper functions for edit distance calculation
    int EditDistance(const std::string& s1, const std::string& s2) const;
    std::vector<std::string> GenerateCandidates(const std::string& word) const;
    bool IsVowel(char c) const;
    
public:
    // Constructors
    Speller(int max_distance = 2);
    Speller(const Speller& other) = default;
    Speller(Speller&& other) = default;
    Speller& operator=(const Speller& other) = default;
    Speller& operator=(Speller&& other) = default;
    ~Speller() = default;
    
    // Dictionary management
    void LoadDictionary(const std::vector<std::string>& words);
    void LoadDictionary(const std::string& filename);
    void AddWord(const std::string& word);
    void RemoveWord(const std::string& word);
    bool ContainsWord(const std::string& word) const;
    int GetDictionarySize() const;
    void ClearDictionary();
    
    // Spell checking
    bool Check(const std::string& word) const;
    std::vector<std::string> Suggest(const std::string& word) const;
    std::string Correct(const std::string& word) const;
    
    // Batch operations
    std::vector<bool> Check(const std::vector<std::string>& words) const;
    std::vector<std::vector<std::string>> Suggest(const std::vector<std::string>& words) const;
    std::vector<std::string> Correct(const std::vector<std::string>& words) const;
    
    // Correction management
    void AddCorrection(const std::string& misspelled, const std::string& corrected);
    void RemoveCorrection(const std::string& misspelled);
    std::vector<std::string> GetCorrections(const std::string& word) const;
    void ClearCorrections();
    
    // Configuration
    void SetMaxEditDistance(int distance);
    int GetMaxEditDistance() const;
    
    // Statistics
    int GetTotalWordsChecked() const;
    int GetTotalCorrectionsMade() const;
    void ResetStatistics();
    
    // Utility functions
    std::vector<std::string> GetAnagrams(const std::string& word) const;
    std::vector<std::string> GetWordsByPrefix(const std::string& prefix) const;
    std::vector<std::string> GetWordsBySuffix(const std::string& suffix) const;
    std::vector<std::string> GetWordsContaining(const std::string& substring) const;
    
    // Case-insensitive variants
    bool CheckIgnoreCase(const std::string& word) const;
    std::vector<std::string> SuggestIgnoreCase(const std::string& word) const;
    std::string CorrectIgnoreCase(const std::string& word) const;
    
    // Locale-specific functions
    void SetLocale(const std::string& locale);
    std::string GetLocale() const;
    
    // Serialization support
    template<typename Stream>
    void Serialize(Stream& s) {
        int version = 1;
        s / version;
        s % max_edit_distance;
        // Serialize dictionary
        if (s.IsStoring()) {
            int dict_size = static_cast<int>(dictionary.size());
            s / dict_size;
            for (const auto& word : dictionary) {
                std::string w = word;
                s % w;
            }
        } else {
            dictionary.clear();
            int dict_size;
            s / dict_size;
            for (int i = 0; i < dict_size; ++i) {
                std::string word;
                s % word;
                dictionary.insert(word);
            }
        }
        // Serialize corrections
        if (s.IsStoring()) {
            int corr_size = static_cast<int>(corrections.size());
            s / corr_size;
            for (const auto& pair : corrections) {
                std::string key = pair.first;
                std::vector<std::string> values = pair.second;
                s % key % values;
            }
        } else {
            corrections.clear();
            int corr_size;
            s / corr_size;
            for (int i = 0; i < corr_size; ++i) {
                std::string key;
                std::vector<std::string> values;
                s % key % values;
                corrections[key] = values;
            }
        }
    }
    
    // Streaming operator
    template<typename Stream>
    friend void operator%(Stream& s, Speller& speller) {
        speller.Serialize(s);
    }
    
    // String representation
    std::string ToString() const;
};

// Global spell checker instance
Speller& GlobalSpeller();

// Convenience functions
inline bool SpellCheck(const std::string& word) {
    return GlobalSpeller().Check(word);
}

inline std::vector<std::string> SpellSuggest(const std::string& word) {
    return GlobalSpeller().Suggest(word);
}

inline std::string SpellCorrect(const std::string& word) {
    return GlobalSpeller().Correct(word);
}

// Batch convenience functions
inline std::vector<bool> SpellCheck(const std::vector<std::string>& words) {
    return GlobalSpeller().Check(words);
}

inline std::vector<std::vector<std::string>> SpellSuggest(const std::vector<std::string>& words) {
    return GlobalSpeller().Suggest(words);
}

inline std::vector<std::string> SpellCorrect(const std::vector<std::string>& words) {
    return GlobalSpeller().Correct(words);
}

// String conversion
inline std::string AsString(const Speller& speller) {
    return speller.ToString();
}

// Implementation details

inline Speller::Speller(int max_distance) : max_edit_distance(max_distance) {}

inline int Speller::EditDistance(const std::string& s1, const std::string& s2) const {
    int len1 = static_cast<int>(s1.length());
    int len2 = static_cast<int>(s2.length());
    
    std::vector<std::vector<int>> dp(len1 + 1, std::vector<int>(len2 + 1));
    
    for (int i = 0; i <= len1; ++i) {
        dp[i][0] = i;
    }
    
    for (int j = 0; j <= len2; ++j) {
        dp[0][j] = j;
    }
    
    for (int i = 1; i <= len1; ++i) {
        for (int j = 1; j <= len2; ++j) {
            if (s1[i - 1] == s2[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1];
            } else {
                dp[i][j] = 1 + std::min({dp[i - 1][j], dp[i][j - 1], dp[i - 1][j - 1]});
            }
        }
    }
    
    return dp[len1][len2];
}

inline std::vector<std::string> Speller::GenerateCandidates(const std::string& word) const {
    std::vector<std::string> candidates;
    
    // Generate deletions
    for (size_t i = 0; i < word.length(); ++i) {
        std::string candidate = word;
        candidate.erase(i, 1);
        candidates.push_back(candidate);
    }
    
    // Generate transpositions
    for (size_t i = 0; i < word.length() - 1; ++i) {
        std::string candidate = word;
        std::swap(candidate[i], candidate[i + 1]);
        candidates.push_back(candidate);
    }
    
    // Generate alterations
    for (size_t i = 0; i < word.length(); ++i) {
        for (char c = 'a'; c <= 'z'; ++c) {
            std::string candidate = word;
            candidate[i] = c;
            candidates.push_back(candidate);
        }
    }
    
    // Generate insertions
    for (size_t i = 0; i <= word.length(); ++i) {
        for (char c = 'a'; c <= 'z'; ++c) {
            std::string candidate = word;
            candidate.insert(i, 1, c);
            candidates.push_back(candidate);
        }
    }
    
    return candidates;
}

inline bool Speller::IsVowel(char c) const {
    char lower_c = std::tolower(c);
    return lower_c == 'a' || lower_c == 'e' || lower_c == 'i' || lower_c == 'o' || lower_c == 'u';
}

inline void Speller::LoadDictionary(const std::vector<std::string>& words) {
    for (const auto& word : words) {
        dictionary.insert(word);
    }
}

inline void Speller::LoadDictionary(const std::string& filename) {
    // Implementation would depend on file I/O capabilities
    // For now, just a placeholder
}

inline void Speller::AddWord(const std::string& word) {
    dictionary.insert(word);
}

inline void Speller::RemoveWord(const std::string& word) {
    dictionary.erase(word);
}

inline bool Speller::ContainsWord(const std::string& word) const {
    return dictionary.find(word) != dictionary.end();
}

inline int Speller::GetDictionarySize() const {
    return static_cast<int>(dictionary.size());
}

inline void Speller::ClearDictionary() {
    dictionary.clear();
}

inline bool Speller::Check(const std::string& word) const {
    return ContainsWord(word);
}

inline std::vector<std::string> Speller::Suggest(const std::string& word) const {
    std::vector<std::string> suggestions;
    std::set<std::string> tried;
    
    // Check for exact match first
    if (ContainsWord(word)) {
        suggestions.push_back(word);
        return suggestions;
    }
    
    // Check for corrections
    auto it = corrections.find(word);
    if (it != corrections.end()) {
        return it->second;
    }
    
    // Generate candidates and check against dictionary
    std::vector<std::string> candidates = GenerateCandidates(word);
    for (const auto& candidate : candidates) {
        if (tried.find(candidate) == tried.end() && ContainsWord(candidate)) {
            suggestions.push_back(candidate);
            tried.insert(candidate);
        }
    }
    
    // Sort by edit distance
    std::sort(suggestions.begin(), suggestions.end(), 
              [this, &word](const std::string& a, const std::string& b) {
                  return EditDistance(word, a) < EditDistance(word, b);
              });
    
    // Limit to reasonable number of suggestions
    if (suggestions.size() > 10) {
        suggestions.resize(10);
    }
    
    return suggestions;
}

inline std::string Speller::Correct(const std::string& word) const {
    std::vector<std::string> suggestions = Suggest(word);
    return suggestions.empty() ? word : suggestions[0];
}

inline std::vector<bool> Speller::Check(const std::vector<std::string>& words) const {
    std::vector<bool> results;
    results.reserve(words.size());
    
    for (const auto& word : words) {
        results.push_back(Check(word));
    }
    
    return results;
}

inline std::vector<std::vector<std::string>> Speller::Suggest(const std::vector<std::string>& words) const {
    std::vector<std::vector<std::string>> results;
    results.reserve(words.size());
    
    for (const auto& word : words) {
        results.push_back(Suggest(word));
    }
    
    return results;
}

inline std::vector<std::string> Speller::Correct(const std::vector<std::string>& words) const {
    std::vector<std::string> results;
    results.reserve(words.size());
    
    for (const auto& word : words) {
        results.push_back(Correct(word));
    }
    
    return results;
}

inline void Speller::AddCorrection(const std::string& misspelled, const std::string& corrected) {
    corrections[misspelled].push_back(corrected);
}

inline void Speller::RemoveCorrection(const std::string& misspelled) {
    corrections.erase(misspelled);
}

inline std::vector<std::string> Speller::GetCorrections(const std::string& word) const {
    auto it = corrections.find(word);
    return it != corrections.end() ? it->second : std::vector<std::string>();
}

inline void Speller::ClearCorrections() {
    corrections.clear();
}

inline void Speller::SetMaxEditDistance(int distance) {
    max_edit_distance = distance;
}

inline int Speller::GetMaxEditDistance() const {
    return max_edit_distance;
}

inline int Speller::GetTotalWordsChecked() const {
    // Placeholder implementation
    return 0;
}

inline int Speller::GetTotalCorrectionsMade() const {
    // Placeholder implementation
    return 0;
}

inline void Speller::ResetStatistics() {
    // Placeholder implementation
}

inline std::vector<std::string> Speller::GetAnagrams(const std::string& word) const {
    std::vector<std::string> anagrams;
    std::string sorted_word = word;
    std::sort(sorted_word.begin(), sorted_word.end());
    
    for (const auto& dict_word : dictionary) {
        if (dict_word.length() == word.length()) {
            std::string sorted_dict_word = dict_word;
            std::sort(sorted_dict_word.begin(), sorted_dict_word.end());
            if (sorted_dict_word == sorted_word) {
                anagrams.push_back(dict_word);
            }
        }
    }
    
    return anagrams;
}

inline std::vector<std::string> Speller::GetWordsByPrefix(const std::string& prefix) const {
    std::vector<std::string> words;
    
    for (const auto& word : dictionary) {
        if (word.length() >= prefix.length() && 
            word.substr(0, prefix.length()) == prefix) {
            words.push_back(word);
        }
    }
    
    return words;
}

inline std::vector<std::string> Speller::GetWordsBySuffix(const std::string& suffix) const {
    std::vector<std::string> words;
    
    for (const auto& word : dictionary) {
        if (word.length() >= suffix.length() && 
            word.substr(word.length() - suffix.length()) == suffix) {
            words.push_back(word);
        }
    }
    
    return words;
}

inline std::vector<std::string> Speller::GetWordsContaining(const std::string& substring) const {
    std::vector<std::string> words;
    
    for (const auto& word : dictionary) {
        if (word.find(substring) != std::string::npos) {
            words.push_back(word);
        }
    }
    
    return words;
}

inline bool Speller::CheckIgnoreCase(const std::string& word) const {
    std::string lower_word = word;
    std::transform(lower_word.begin(), lower_word.end(), lower_word.begin(), ::tolower);
    return Check(lower_word);
}

inline std::vector<std::string> Speller::SuggestIgnoreCase(const std::string& word) const {
    std::string lower_word = word;
    std::transform(lower_word.begin(), lower_word.end(), lower_word.begin(), ::tolower);
    return Suggest(lower_word);
}

inline std::string Speller::CorrectIgnoreCase(const std::string& word) const {
    std::string lower_word = word;
    std::transform(lower_word.begin(), lower_word.end(), lower_word.begin(), ::tolower);
    return Correct(lower_word);
}

inline void Speller::SetLocale(const std::string& locale) {
    // Placeholder implementation
}

inline std::string Speller::GetLocale() const {
    // Placeholder implementation
    return "en_US";
}

inline std::string Speller::ToString() const {
    return "Speller(dictionary=" + std::to_string(dictionary.size()) + 
           ", corrections=" + std::to_string(corrections.size()) + ")";
}

// Global spell checker instance
inline Speller& GlobalSpeller() {
    static Speller speller;
    return speller;
}

#endif