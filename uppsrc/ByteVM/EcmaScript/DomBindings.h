#ifndef _ByteVM_EcmaScript_DomBindings_h_
#define _ByteVM_EcmaScript_DomBindings_h_

#include "JsBindings.h"

NAMESPACE_UPP
#include <Core/Inet.h>
END_UPP_NAMESPACE

NAMESPACE_UPP

void InitDomBindings(JsVM& vm);

END_UPP_NAMESPACE

#endif
