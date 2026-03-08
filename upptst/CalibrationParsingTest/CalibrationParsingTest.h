#ifndef _CalibrationParsingTest_CalibrationParsingTest_h_
#define _CalibrationParsingTest_CalibrationParsingTest_h_

#include <Core/Core.h>

NAMESPACE_UPP

// Test locale-independent parsing
bool TestParseDoubleLocaleIndependent();

// Test .stcal round-trip
bool TestStcalRoundTrip();

END_UPP_NAMESPACE

#endif
