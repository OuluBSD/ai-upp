#ifndef _Node_Script_EonIO_h_
#define _Node_Script_EonIO_h_

#include <Node/Core/Core.h>

namespace Upp {
namespace Node {

// Load an extended .eon text into a Graph.
// Populates NodeDoc (id, label, params via slots), EdgeDoc (source/target with named pins),
// and PinDoc entries from 'node' type declarations.
// Returns false on parse or structural errors; error details appended to 'out'.
bool LoadEon(Graph& g, const String& eon_text,
             Vector<ValidationMessage>& out);

// Convenience: load from a file path.
bool LoadEonFile(Graph& g, const String& path,
                 Vector<ValidationMessage>& out);

// Serialize a Graph back to extended .eon text.
// Emits 'node' type declarations (for nodes that have pins) followed by a 'net' block.
String SaveEon(const Graph& g);

} // namespace Node
} // namespace Upp

#endif
