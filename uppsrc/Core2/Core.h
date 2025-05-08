#ifndef _Core2_Core_h_
#define _Core2_Core_h_

#include <Core/Core.h>

#ifdef flagFREEBSD
extern char **environ;
#endif

NAMESPACE_UPP

#include "Compat.h"
#include "TypeTraits.h"
#include "String.h"
#include "Ctrl.h"
#include "Util.h"
#include "Process.h"
#include "Tokenizer.h"
#include "Container.h"
#include "Index.h"
#include "DCT.h"
#include "Coordinate.h"
#include "Chrono.h"
#include "Record.h"
#include "Color.h"
#include "Geom.h"
#include "Html.h"
#include "Url.h"
#include "Crypto.h"
#include "TokenParser.h"
#include "RTuple.h"
#include "Random.h"
#include "CKMeans.h"

END_UPP_NAMESPACE

#endif
