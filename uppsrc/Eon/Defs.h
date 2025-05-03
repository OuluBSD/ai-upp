#ifndef _Eon_Defs_h_
#define _Eon_Defs_h_

// If "debug realtime errors" is defined (with DEBUG_RT)
#ifdef flagDEBUG_RT
	#define DEBUG_RT_PIPE 1
	#ifdef flagDEBUG
		#define RTLOG(x) DLOG(x)
	#else
		#define RTLOG(x) RLOG(x)
	#endif
#else
	#define DEBUG_RT_PIPE 0
	#define RTLOG(x) {}
#endif

#ifdef flagDEBUG
	#define DEFAULT_AUDIO_QUEUE_SIZE	10
#else
	#define DEFAULT_AUDIO_QUEUE_SIZE	4
#endif

using EntityId				= int32;
using PoolId				= int32;

#endif
