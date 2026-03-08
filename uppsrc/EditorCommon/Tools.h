#ifndef _CardEngine_Tools_h_
#define _CardEngine_Tools_h_

#include <Core/Core.h>

NAMESPACE_UPP

class Tools
{
public:
	static void SeedRandom(unsigned seed);
	static void ShuffleArrayNonDeterministic(int *inout, int count);
	static void GetRand(int minValue, int maxValue, int count, int *out);
	static String GetDataDir();
};

END_UPP_NAMESPACE

#endif
