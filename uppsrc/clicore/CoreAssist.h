#ifndef _clicore_CoreAssist_h_
#define _clicore_CoreAssist_h_

#include <clicore/clicore.h>

class CoreAssist : Moveable<CoreAssist> {
public:
    CoreAssist();

    // Index management
    bool IndexWorkspace(const CoreWorkspace& ws, String& error);
    bool IndexFile(const String& path, String& error);

    // Queries
    bool FindDefinition(const String& symbol, String& out_file, int& out_line) const;
    bool FindUsages(const String& symbol, Vector<String>& out_locations) const;

    // Helpers
    bool ExtractSymbols(const String& content,
                        Vector<String>& out_symbols,
                        Vector<String>& out_includes) const;

private:
    struct SymbolLocation : Moveable<SymbolLocation> {
        String file;
        int line;
    };

    // symbol → definition location
    ArrayMap<String, SymbolLocation> definitions;

    // symbol → list of usage locations
    VectorMap<String, Vector<SymbolLocation>> usages;
};

#endif