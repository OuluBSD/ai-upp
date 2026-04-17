#ifndef _Node_Core_Spiral_h_
#define _Node_Core_Spiral_h_

#include "Core.h"

namespace Upp {

namespace Node {

// Returns n positions arranged on an Archimedean spiral centred at (0,0).
// spacing: arc-length between successive turns.
Vector<Pointf> SpiralPositions(int n, double spacing);

// Returns n positions uniformly arranged on a circle centred at (0,0).
// radius: circle radius; if 0, computed automatically from n and spacing.
Vector<Pointf> CirclePositions(int n, double spacing, double radius = 0.0);

} // namespace Node

} // namespace Upp

#endif
