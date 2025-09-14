#ifndef _SoftAudioGraph_Connection_h_
#define _SoftAudioGraph_Connection_h_

#include <Core2/Core.h>

#ifndef NAMESPACE_SAGRAPH_BEGIN
#define NAMESPACE_SAGRAPH_BEGIN NAMESPACE_UPP namespace SAGraph {
#define NAMESPACE_SAGRAPH_END   END_UPP_NAMESPACE }
#endif

NAMESPACE_SAGRAPH_BEGIN

struct Edge {
    int from = -1; // node index
    int from_port = 0;
    int to = -1;   // node index
    int to_port = 0;
    float gain = 1.0f; // per-connection gain
};

NAMESPACE_SAGRAPH_END

#endif

