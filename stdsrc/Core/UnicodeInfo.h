#pragma once
#ifndef _Core_UnicodeInfo_h_
#define _Core_UnicodeInfo_h_

#include <string>
#include <vector>
#include <cstdint>
#include "Core.h"

// Unicode character information and utilities for stdsrc

class UnicodeInfo {
private:
    // Unicode character properties
    struct CharInfo {
        uint32_t codepoint;
        uint8_t category;      // Unicode general category
        uint8_t bidi_class;    // Bidirectional class
        uint8_t decomposition_type;
        uint8_t numeric_type;
        int8_t  combining_class;
        bool    is_uppercase : 1;
        bool    is_lowercase : 1;
        bool    is_titlecase : 1;
        bool    is_mirror : 1;
        bool    is_white_space : 1;
        bool    is_dash : 1;
        bool    is_hyphen : 1;
        bool    is_quotation_mark : 1;
        bool    is_terminal_punctuation : 1;
        bool    is_other_math : 1;
        bool    is_hex_digit : 1;
        bool    is_ascii_hex_digit : 1;
        bool    is_other_alphabetic : 1;
        bool    is_ideographic : 1;
        bool    is_diacritic : 1;
        bool    is_extender : 1;
        bool    is_other_lowercase : 1;
        bool    is_other_uppercase : 1;
        bool    is_noncharacter : 1;
        bool    is_default_ignorable : 1;
        bool    is_logical_order_exception : 1;
        bool    is_prepended_concatenation_mark : 1;
        
        CharInfo() : codepoint(0), category(0), bidi_class(0), decomposition_type(0), 
                     numeric_type(0), combining_class(0), is_uppercase(false), 
                     is_lowercase(false), is_titlecase(false), is_mirror(false),
                     is_white_space(false), is_dash(false), is_hyphen(false),
                     is_quotation_mark(false), is_terminal_punctuation(false),
                     is_other_math(false), is_hex_digit(false), is_ascii_hex_digit(false),
                     is_other_alphabetic(false), is_ideographic(false), is_diacritic(false),
                     is_extender(false), is_other_lowercase(false), is_other_uppercase(false),
                     is_noncharacter(false), is_default_ignorable(false),
                     is_logical_order_exception(false), is_prepended_concatenation_mark(false) {}
    };
    
    // Unicode categories
    enum Category {
        UNASSIGNED = 0,
        UPPERCASE_LETTER = 1,
        LOWERCASE_LETTER = 2,
        TITLECASE_LETTER = 3,
        MODIFIER_LETTER = 4,
        OTHER_LETTER = 5,
        NON_SPACING_MARK = 6,
        ENCLOSING_MARK = 7,
        COMBINING_SPACING_MARK = 8,
        DECIMAL_DIGIT_NUMBER = 9,
        LETTER_NUMBER = 10,
        OTHER_NUMBER = 11,
        SPACE_SEPARATOR = 12,
        LINE_SEPARATOR = 13,
        PARAGRAPH_SEPARATOR = 14,
        CONTROL = 15,
        FORMAT = 16,
        PRIVATE_USE = 17,
        SURROGATE = 18,
        DASH_PUNCTUATION = 19,
        START_PUNCTUATION = 20,
        END_PUNCTUATION = 21,
        CONNECTOR_PUNCTUATION = 22,
        OTHER_PUNCTUATION = 23,
        MATH_SYMBOL = 24,
        CURRENCY_SYMBOL = 25,
        MODIFIER_SYMBOL = 26,
        OTHER_SYMBOL = 27,
        INITIAL_PUNCTUATION = 28,
        FINAL_PUNCTUATION = 29
    };
    
    // Bidirectional classes
    enum BidiClass {
        L = 0,    // Left-to-Right
        LRE = 1,  // Left-to-Right Embedding
        LRO = 2,  // Left-to-Right Override
        R = 3,    // Right-to-Left
        AL = 4,   // Right-to-Left Arabic
        RLE = 5,  // Right-to-Left Embedding
        RLO = 6,  // Right-to-Left Override
        PDF = 7,  // Pop Directional Format
        EN = 8,   // European Number
        ES = 9,   // European Number Separator
        ET = 10,  // European Number Terminator
        AN = 11,  // Arabic Number
        CS = 12,  // Common Number Separator
        NSM = 13, // Non-Spacing Mark
        BN = 14,  // Boundary Neutral
        B = 15,   // Paragraph Separator
        S = 16,   // Segment Separator
        WS = 17,  // Whitespace
        ON = 18,  // Other Neutrals
        LRI = 19, // Left-to-Right Isolate
        RLI = 20, // Right-to-Left Isolate
        FSI = 21, // First Strong Isolate
        PDI = 22  // Pop Directional Isolate
    };
    
    // Decomposition types
    enum DecompositionType {
        DT_NONE = 0,
        DT_CANONICAL = 1,
        DT_FONT = 2,
        DT_NOBREAK = 3,
        DT_INITIAL = 4,
        DT_MEDIAL = 5,
        DT_FINAL = 6,
        DT_ISOLATED = 7,
        DT_CIRCLE = 8,
        DT_SUPER = 9,
        DT_SUB = 10,
        DT_VERTICAL = 11,
        DT_WIDE = 12,
        DT_NARROW = 13,
        DT_SMALL = 14,
        DT_SQUARE = 15,
        DT_FRACTION = 16,
        DT_COMPAT = 17
    };
    
    // Numeric types
    enum NumericType {
        NT_NONE = 0,
        NT_DECIMAL = 1,
        NT_DIGIT = 2,
        NT_NUMERIC = 3
    };
    
    // Private data
    static std::vector<CharInfo> char_info_table;
    static bool initialized;
    
    // Helper functions
    static void Initialize();
    static const CharInfo& GetCharInfo(uint32_t codepoint);
    static uint32_t GetSimpleCaseMapping(uint32_t codepoint, int mapping_type);
    
public:
    // Character category functions
    static bool IsLetter(uint32_t codepoint);
    static bool IsUpperCase(uint32_t codepoint);
    static bool IsLowerCase(uint32_t codepoint);
    static bool IsTitleCase(uint32_t codepoint);
    static bool IsDigit(uint32_t codepoint);
    static bool IsNumber(uint32_t codepoint);
    static bool IsSymbol(uint32_t codepoint);
    static bool IsPunctuation(uint32_t codepoint);
    static bool IsSeparator(uint32_t codepoint);
    static bool IsControl(uint32_t codepoint);
    static bool IsMark(uint32_t codepoint);
    static bool IsPrivateUse(uint32_t codepoint);
    static bool IsSurrogate(uint32_t codepoint);
    static bool IsNonCharacter(uint32_t codepoint);
    
    // White space and related functions
    static bool IsWhiteSpace(uint32_t codepoint);
    static bool IsDash(uint32_t codepoint);
    static bool IsHyphen(uint32_t codepoint);
    static bool IsQuotationMark(uint32_t codepoint);
    static bool IsTerminalPunctuation(uint32_t codepoint);
    
    // Mathematical and formatting functions
    static bool IsMath(uint32_t codepoint);
    static bool IsCurrency(uint32_t codepoint);
    static bool IsModifier(uint32_t codepoint);
    static bool IsOther(uint32_t codepoint);
    
    // Hexadecimal digit functions
    static bool IsHexDigit(uint32_t codepoint);
    static bool IsAsciiHexDigit(uint32_t codepoint);
    
    // Bidirectional functions
    static int GetBidiClass(uint32_t codepoint);
    static bool IsMirrored(uint32_t codepoint);
    
    // Case mapping functions
    static uint32_t ToUpper(uint32_t codepoint);
    static uint32_t ToLower(uint32_t codepoint);
    static uint32_t ToTitle(uint32_t codepoint);
    static std::string ToUpper(const std::string& str);
    static std::string ToLower(const std::string& str);
    static std::string ToTitle(const std::string& str);
    
    // Numeric functions
    static int GetNumericType(uint32_t codepoint);
    static int GetNumericValue(uint32_t codepoint);
    static double GetNumericDoubleValue(uint32_t codepoint);
    
    // Decomposition functions
    static int GetDecompositionType(uint32_t codepoint);
    static std::vector<uint32_t> GetDecomposition(uint32_t codepoint);
    static bool IsCanonical(uint32_t codepoint);
    static bool IsCompatible(uint32_t codepoint);
    
    // Combining class functions
    static int GetCombiningClass(uint32_t codepoint);
    static bool IsCombiningMark(uint32_t codepoint);
    
    // Script and block functions
    static std::string GetScript(uint32_t codepoint);
    static std::string GetBlock(uint32_t codepoint);
    
    // Age and version functions
    static std::string GetAge(uint32_t codepoint);
    static int GetVersion(uint32_t codepoint);
    
    // East Asian width functions
    static int GetEastAsianWidth(uint32_t codepoint);
    static bool IsFullWidth(uint32_t codepoint);
    static bool IsHalfWidth(uint32_t codepoint);
    static bool IsWide(uint32_t codepoint);
    static bool IsNarrow(uint32_t codepoint);
    static bool IsAmbiguous(uint32_t codepoint);
    static bool IsNeutral(uint32_t codepoint);
    
    // Line breaking functions
    static int GetLineBreak(uint32_t codepoint);
    static bool IsLineBreakable(uint32_t codepoint);
    
    // Joining type functions
    static int GetJoiningType(uint32_t codepoint);
    static int GetJoiningGroup(uint32_t codepoint);
    
    // Emoji functions
    static bool IsEmoji(uint32_t codepoint);
    static bool IsEmojiPresentation(uint32_t codepoint);
    static bool IsEmojiModifier(uint32_t codepoint);
    static bool IsEmojiModifierBase(uint32_t codepoint);
    static bool IsEmojiComponent(uint32_t codepoint);
    
    // Hangul functions
    static bool IsHangulSyllable(uint32_t codepoint);
    static bool IsHangulJamo(uint32_t codepoint);
    static bool IsHangulCompatibilityJamo(uint32_t codepoint);
    static bool IsHangulJamoExtendedA(uint32_t codepoint);
    static bool IsHangulJamoExtendedB(uint32_t codepoint);
    
    // CJK functions
    static bool IsCJKIdeograph(uint32_t codepoint);
    static bool IsCJKUnifiedIdeograph(uint32_t codepoint);
    static bool IsCJKCompatibilityIdeograph(uint32_t codepoint);
    static bool IsCJKRadicalSupplement(uint32_t codepoint);
    static bool IsCJKStrokes(uint32_t codepoint);
    static bool IsCJKCompatibilityForms(uint32_t codepoint);
    
    // Indic functions
    static bool IsIndicSyllabicCategory(uint32_t codepoint);
    static bool IsIndicMatraCategory(uint32_t codepoint);
    
    // Utility functions
    static std::string GetCharacterName(uint32_t codepoint);
    static std::string GetPropertyValue(uint32_t codepoint, const std::string& property);
    static std::vector<std::string> GetPropertyAliases(const std::string& property);
    static std::vector<std::string> GetPropertyValueAliases(const std::string& property, const std::string& value);
    
    // Range functions
    static std::vector<uint32_t> GetCharactersInRange(uint32_t start, uint32_t end);
    static std::vector<uint32_t> GetCharactersInCategory(int category);
    static std::vector<uint32_t> GetCharactersInScript(const std::string& script);
    static std::vector<uint32_t> GetCharactersInBlock(const std::string& block);
    
    // Validation functions
    static bool IsValidCodepoint(uint32_t codepoint);
    static bool IsAssignedCodepoint(uint32_t codepoint);
    static bool IsReservedCodepoint(uint32_t codepoint);
    
    // String representation
    static std::string ToString(uint32_t codepoint);
    static std::string ToString() { return "UnicodeInfo"; }
    
    // Streaming operator
    template<typename Stream>
    friend void operator%(Stream& s, UnicodeInfo& info) {
        // UnicodeInfo is stateless, so nothing to serialize
    }
    
    // Constructor and destructor
    UnicodeInfo() { Initialize(); }
    ~UnicodeInfo() = default;
};

// Global Unicode information functions
inline bool IsLetter(uint32_t codepoint) { return UnicodeInfo::IsLetter(codepoint); }
inline bool IsUpperCase(uint32_t codepoint) { return UnicodeInfo::IsUpperCase(codepoint); }
inline bool IsLowerCase(uint32_t codepoint) { return UnicodeInfo::IsLowerCase(codepoint); }
inline bool IsTitleCase(uint32_t codepoint) { return UnicodeInfo::IsTitleCase(codepoint); }
inline bool IsDigit(uint32_t codepoint) { return UnicodeInfo::IsDigit(codepoint); }
inline bool IsNumber(uint32_t codepoint) { return UnicodeInfo::IsNumber(codepoint); }
inline bool IsSymbol(uint32_t codepoint) { return UnicodeInfo::IsSymbol(codepoint); }
inline bool IsPunctuation(uint32_t codepoint) { return UnicodeInfo::IsPunctuation(codepoint); }
inline bool IsSeparator(uint32_t codepoint) { return UnicodeInfo::IsSeparator(codepoint); }
inline bool IsControl(uint32_t codepoint) { return UnicodeInfo::IsControl(codepoint); }
inline bool IsMark(uint32_t codepoint) { return UnicodeInfo::IsMark(codepoint); }
inline bool IsPrivateUse(uint32_t codepoint) { return UnicodeInfo::IsPrivateUse(codepoint); }
inline bool IsSurrogate(uint32_t codepoint) { return UnicodeInfo::IsSurrogate(codepoint); }
inline bool IsNonCharacter(uint32_t codepoint) { return UnicodeInfo::IsNonCharacter(codepoint); }

inline bool IsWhiteSpace(uint32_t codepoint) { return UnicodeInfo::IsWhiteSpace(codepoint); }
inline bool IsDash(uint32_t codepoint) { return UnicodeInfo::IsDash(codepoint); }
inline bool IsHyphen(uint32_t codepoint) { return UnicodeInfo::IsHyphen(codepoint); }
inline bool IsQuotationMark(uint32_t codepoint) { return UnicodeInfo::IsQuotationMark(codepoint); }
inline bool IsTerminalPunctuation(uint32_t codepoint) { return UnicodeInfo::IsTerminalPunctuation(codepoint); }

inline bool IsMath(uint32_t codepoint) { return UnicodeInfo::IsMath(codepoint); }
inline bool IsCurrency(uint32_t codepoint) { return UnicodeInfo::IsCurrency(codepoint); }
inline bool IsModifier(uint32_t codepoint) { return UnicodeInfo::IsModifier(codepoint); }
inline bool IsOther(uint32_t codepoint) { return UnicodeInfo::IsOther(codepoint); }

inline bool IsHexDigit(uint32_t codepoint) { return UnicodeInfo::IsHexDigit(codepoint); }
inline bool IsAsciiHexDigit(uint32_t codepoint) { return UnicodeInfo::IsAsciiHexDigit(codepoint); }

inline int GetBidiClass(uint32_t codepoint) { return UnicodeInfo::GetBidiClass(codepoint); }
inline bool IsMirrored(uint32_t codepoint) { return UnicodeInfo::IsMirrored(codepoint); }

inline uint32_t ToUpper(uint32_t codepoint) { return UnicodeInfo::ToUpper(codepoint); }
inline uint32_t ToLower(uint32_t codepoint) { return UnicodeInfo::ToLower(codepoint); }
inline uint32_t ToTitle(uint32_t codepoint) { return UnicodeInfo::ToTitle(codepoint); }
inline std::string ToUpper(const std::string& str) { return UnicodeInfo::ToUpper(str); }
inline std::string ToLower(const std::string& str) { return UnicodeInfo::ToLower(str); }
inline std::string ToTitle(const std::string& str) { return UnicodeInfo::ToTitle(str); }

inline int GetNumericType(uint32_t codepoint) { return UnicodeInfo::GetNumericType(codepoint); }
inline int GetNumericValue(uint32_t codepoint) { return UnicodeInfo::GetNumericValue(codepoint); }
inline double GetNumericDoubleValue(uint32_t codepoint) { return UnicodeInfo::GetNumericDoubleValue(codepoint); }

inline int GetDecompositionType(uint32_t codepoint) { return UnicodeInfo::GetDecompositionType(codepoint); }
inline std::vector<uint32_t> GetDecomposition(uint32_t codepoint) { return UnicodeInfo::GetDecomposition(codepoint); }
inline bool IsCanonical(uint32_t codepoint) { return UnicodeInfo::IsCanonical(codepoint); }
inline bool IsCompatible(uint32_t codepoint) { return UnicodeInfo::IsCompatible(codepoint); }

inline int GetCombiningClass(uint32_t codepoint) { return UnicodeInfo::GetCombiningClass(codepoint); }
inline bool IsCombiningMark(uint32_t codepoint) { return UnicodeInfo::IsCombiningMark(codepoint); }

inline std::string GetScript(uint32_t codepoint) { return UnicodeInfo::GetScript(codepoint); }
inline std::string GetBlock(uint32_t codepoint) { return UnicodeInfo::GetBlock(codepoint); }

inline std::string GetAge(uint32_t codepoint) { return UnicodeInfo::GetAge(codepoint); }
inline int GetVersion(uint32_t codepoint) { return UnicodeInfo::GetVersion(codepoint); }

inline int GetEastAsianWidth(uint32_t codepoint) { return UnicodeInfo::GetEastAsianWidth(codepoint); }
inline bool IsFullWidth(uint32_t codepoint) { return UnicodeInfo::IsFullWidth(codepoint); }
inline bool IsHalfWidth(uint32_t codepoint) { return UnicodeInfo::IsHalfWidth(codepoint); }
inline bool IsWide(uint32_t codepoint) { return UnicodeInfo::IsWide(codepoint); }
inline bool IsNarrow(uint32_t codepoint) { return UnicodeInfo::IsNarrow(codepoint); }
inline bool IsAmbiguous(uint32_t codepoint) { return UnicodeInfo::IsAmbiguous(codepoint); }
inline bool IsNeutral(uint32_t codepoint) { return UnicodeInfo::IsNeutral(codepoint); }

inline int GetLineBreak(uint32_t codepoint) { return UnicodeInfo::GetLineBreak(codepoint); }
inline bool IsLineBreakable(uint32_t codepoint) { return UnicodeInfo::IsLineBreakable(codepoint); }

inline int GetJoiningType(uint32_t codepoint) { return UnicodeInfo::GetJoiningType(codepoint); }
inline int GetJoiningGroup(uint32_t codepoint) { return UnicodeInfo::GetJoiningGroup(codepoint); }

inline bool IsEmoji(uint32_t codepoint) { return UnicodeInfo::IsEmoji(codepoint); }
inline bool IsEmojiPresentation(uint32_t codepoint) { return UnicodeInfo::IsEmojiPresentation(codepoint); }
inline bool IsEmojiModifier(uint32_t codepoint) { return UnicodeInfo::IsEmojiModifier(codepoint); }
inline bool IsEmojiModifierBase(uint32_t codepoint) { return UnicodeInfo::IsEmojiModifierBase(codepoint); }
inline bool IsEmojiComponent(uint32_t codepoint) { return UnicodeInfo::IsEmojiComponent(codepoint); }

inline bool IsHangulSyllable(uint32_t codepoint) { return UnicodeInfo::IsHangulSyllable(codepoint); }
inline bool IsHangulJamo(uint32_t codepoint) { return UnicodeInfo::IsHangulJamo(codepoint); }
inline bool IsHangulCompatibilityJamo(uint32_t codepoint) { return UnicodeInfo::IsHangulCompatibilityJamo(codepoint); }
inline bool IsHangulJamoExtendedA(uint32_t codepoint) { return UnicodeInfo::IsHangulJamoExtendedA(codepoint); }
inline bool IsHangulJamoExtendedB(uint32_t codepoint) { return UnicodeInfo::IsHangulJamoExtendedB(codepoint); }

inline bool IsCJKIdeograph(uint32_t codepoint) { return UnicodeInfo::IsCJKIdeograph(codepoint); }
inline bool IsCJKUnifiedIdeograph(uint32_t codepoint) { return UnicodeInfo::IsCJKUnifiedIdeograph(codepoint); }
inline bool IsCJKCompatibilityIdeograph(uint32_t codepoint) { return UnicodeInfo::IsCJKCompatibilityIdeograph(codepoint); }
inline bool IsCJKRadicalSupplement(uint32_t codepoint) { return UnicodeInfo::IsCJKRadicalSupplement(codepoint); }
inline bool IsCJKStrokes(uint32_t codepoint) { return UnicodeInfo::IsCJKStrokes(codepoint); }
inline bool IsCJKCompatibilityForms(uint32_t codepoint) { return UnicodeInfo::IsCJKCompatibilityForms(codepoint); }

inline bool IsIndicSyllabicCategory(uint32_t codepoint) { return UnicodeInfo::IsIndicSyllabicCategory(codepoint); }
inline bool IsIndicMatraCategory(uint32_t codepoint) { return UnicodeInfo::IsIndicMatraCategory(codepoint); }

inline std::string GetCharacterName(uint32_t codepoint) { return UnicodeInfo::GetCharacterName(codepoint); }
inline std::string GetPropertyValue(uint32_t codepoint, const std::string& property) { return UnicodeInfo::GetPropertyValue(codepoint, property); }
inline std::vector<std::string> GetPropertyAliases(const std::string& property) { return UnicodeInfo::GetPropertyAliases(property); }
inline std::vector<std::string> GetPropertyValueAliases(const std::string& property, const std::string& value) { return UnicodeInfo::GetPropertyValueAliases(property, value); }

inline std::vector<uint32_t> GetCharactersInRange(uint32_t start, uint32_t end) { return UnicodeInfo::GetCharactersInRange(start, end); }
inline std::vector<uint32_t> GetCharactersInCategory(int category) { return UnicodeInfo::GetCharactersInCategory(category); }
inline std::vector<uint32_t> GetCharactersInScript(const std::string& script) { return UnicodeInfo::GetCharactersInScript(script); }
inline std::vector<uint32_t> GetCharactersInBlock(const std::string& block) { return UnicodeInfo::GetCharactersInBlock(block); }

inline bool IsValidCodepoint(uint32_t codepoint) { return UnicodeInfo::IsValidCodepoint(codepoint); }
inline bool IsAssignedCodepoint(uint32_t codepoint) { return UnicodeInfo::IsAssignedCodepoint(codepoint); }
inline bool IsReservedCodepoint(uint32_t codepoint) { return UnicodeInfo::IsReservedCodepoint(codepoint); }

inline std::string ToString(uint32_t codepoint) { return UnicodeInfo::ToString(codepoint); }
inline std::string ToString(const UnicodeInfo& info) { return info.ToString(); }

// Global Unicode information instance
inline UnicodeInfo& GlobalUnicodeInfo() {
    static UnicodeInfo instance;
    return instance;
}

#endif