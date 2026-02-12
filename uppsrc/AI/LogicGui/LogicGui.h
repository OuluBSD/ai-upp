#ifndef _AI_LogicGui_LogicGui_h_
#define _AI_LogicGui_LogicGui_h_

#include "ConstraintVisitor.h"

namespace Upp {

extern Event<String, bool> WhenCheckConstraintsResult;

void LinkLogicGui();

}

#endif
