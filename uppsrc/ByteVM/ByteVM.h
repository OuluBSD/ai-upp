#ifndef _ByteVM_ByteVM_h_
#define _ByteVM_ByteVM_h_

#include <Core/Core.h>
#include <Core/TextParsing/TextParsing.h>

#ifndef RTLOG
#ifdef flagDEBUG_RT
#define RTLOG(x) DLOG(x)
#else
#define RTLOG(x)
#endif
#endif

#include "PyCode.h"
#include "PyValue.h"
#include "PyIR.h"
#include "PyCompiler.h"
#include "PyVM.h"
#include "PolicyKit.h"
#include "PyScheduler.h"

#endif
