#include "DbgCrashSmoke.h"

using namespace Upp;

#if defined(_MSC_VER)
#define NOINLINE __declspec(noinline)
#elif defined(__GNUC__)
#define NOINLINE __attribute__((noinline))
#else
#define NOINLINE
#endif

static NOINLINE void CrashSmokeCrash()
{
	volatile int *p = nullptr;
	*p = 1;
}

static NOINLINE void CrashSmokeLevel2()
{
	CrashSmokeCrash();
}

static NOINLINE void CrashSmokeLevel1()
{
	CrashSmokeLevel2();
}

static NOINLINE void CrashSmokeMain()
{
	CrashSmokeLevel1();
}

static void PrintHelp()
{
	Cout() << "Usage: DbgCrashSmoke [--help] [--no-crash] [--crash]\n"
	       << "  --help      Show this help text.\n"
	       << "  --no-crash  Run the zero-exit sanity path.\n"
	       << "  --crash     Force the intentional crash path.\n"
	       << "\n"
	       << "Default behavior with no arguments is the intentional crash path.\n";
}

CONSOLE_APP_MAIN
{
	const Vector<String>& args = CommandLine();
	if(args.IsEmpty()) {
		CrashSmokeMain();
		return;
	}

	if(FindIndex(args, "--help") >= 0 || FindIndex(args, "-h") >= 0) {
		PrintHelp();
		return;
	}

	if(FindIndex(args, "--no-crash") >= 0) {
		Cout() << "DbgCrashSmoke: sanity path\n";
		return;
	}

	if(FindIndex(args, "--crash") >= 0) {
		CrashSmokeMain();
		return;
	}

	Cerr() << "DbgCrashSmoke: unknown arguments\n";
	PrintHelp();
	SetExitCode(1);
}
