#ifndef CMD_IDEHOST_H
#define CMD_IDEHOST_H

#include "cli.h"
#include <Core/Core.h>

#ifdef flagGUI
#include "ide/ide.h"

namespace Upp {

class IdeHost {
public:
    IdeHost();
    ~IdeHost();

    bool Init(String& error);
    Ide& GetIde();
    const Ide& GetIde() const;
    
private:
    One<Ide> ide;
    bool initialized;
    
    bool InitializeLanguageAndCharset(String& error);
    bool InitializeConfig(String& error);
    bool InitializeIdeInstance(String& error);
};

IdeHost& GetGlobalIdeHost();

}
#endif

#endif