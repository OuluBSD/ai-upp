#ifndef RAINBOWGAME_STRINGS_H
#define RAINBOWGAME_STRINGS_H

#include <Core/Core.h>

using namespace Upp;

class Strings {
public:
    Strings();
    ~Strings();
    
    String Get(const String& key) const;
    String GetWithParams(const String& key, const Vector<Value>& params) const;
    
    void SetLanguage(const String& languageCode);
    String GetLanguage() const;
    
    void LoadStrings(const String& languageCode);
    
private:
    String languageCode;
    ValueMap strings;
};

#endif