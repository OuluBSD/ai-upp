#include "CalibrationParsingTest.h"

using namespace Upp;

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT|LOG_FILE);

	Cout() << "========================================\n";
	Cout() << "Calibration Parsing Tests\n";
	Cout() << "========================================\n\n";

	bool all_ok = true;

	Cout() << "Test 1: ParseDoubleLocaleIndependent\n";
	Cout() << "-------------------------------------\n";
	if (!TestParseDoubleLocaleIndependent()) {
		all_ok = false;
		Cout() << "\nTest 1 FAILED\n\n";
	} else {
		Cout() << "\nTest 1 PASSED\n\n";
	}

	Cout() << "Test 2: .stcal Round-trip\n";
	Cout() << "-------------------------\n";
	if (!TestStcalRoundTrip()) {
		all_ok = false;
		Cout() << "\nTest 2 FAILED\n\n";
	} else {
		Cout() << "\nTest 2 PASSED\n\n";
	}

	if (all_ok) {
		Cout() << "========================================\n";
		Cout() << "ALL TESTS PASSED\n";
		Cout() << "========================================\n";
		SetExitCode(0);
	} else {
		Cout() << "========================================\n";
		Cout() << "SOME TESTS FAILED\n";
		Cout() << "========================================\n";
		SetExitCode(1);
	}
}
