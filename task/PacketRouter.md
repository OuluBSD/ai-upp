# Packet Router Task Thread

## Overview
Convert the current loop-based Eon system to a router-based system where atoms connect to a router system instead of forming loops. This transformation will involve morphing Exchange classes to function as packet routers.

## Key Requirements

### System Architecture Changes
- Convert the loop-based Eon system to a router-based architecture
- Instead of connecting atoms in loops, implement a router system for connections
- Transform Exchange classes to serve as packet routers

### Codebase Analysis
- Analyze uppsrc/Eon packages to understand current implementation
- Identify other relevant areas in the codebase that will be affected
- Document current atom connection patterns and loop mechanisms

### File Modifications
- Modify all *.eon files in share/eon (deep analysis and changes required)
- Update all upptst/Eon* packages to align with new router-based system

### Development Roadmap
- Plan the implementation roadmap for this extensive conversion
- This is a long-term process requiring careful planning and execution
- Consider backward compatibility and transition strategies

## Implementation Phases
(To be detailed after initial analysis)

## Dependencies
- Understanding of current Eon system architecture
- Knowledge of atom connection mechanisms
- Understanding of Exchange class functionality

## Purpose of Current Loop System
- The whole point of the loop system was to even out the packets in the loop
- This allowed the flow of packet signals demand between atoms

## Flow Control Requirements
- The demand between packets needs to be addressed in a different way in the new architecture
- We might add ports for atoms and connect some ports as signals
- "Port" will replace the concept of primary sink/src (id=0) and secondary sink/src
- All ports will be equal in rank (we will discard the "primary" qualifier)
- We will still have src/sink ports, connecting output to input
- Sending packets to ports won't require sending packets through a primary port anymore
- The "sync" sending feature will be maintained inside packets using a helper class or attribute
- This sync feature will not be enforced at the architecture level anymore in the new version
- We could implement the old loop system in the new version using the new flow-control signal
- Uncertainty remains about the flow-control logic: previous system used constant amount of packets in the loop
- We might still need some sort of virtual pool of packets for groups of atoms

## Morphed Exchange Classes
- The new routing system will require morphing existing Exchange classes to function as packet routers
- `ExchangeSourceProvider` → `RouterSourceProvider` (handles output ports in the new system)
- `ExchangeSinkProvider` → `RouterSinkProvider` (handles input ports in the new system)
- `ExchangePoint` → `RouterConnection` (handles routing between ports instead of direct connections)
- `ExchangeSideSourceProvider` → `RouterSideSourceProvider` (handles side connections)
- `ExchangeSideSinkProvider` → `RouterSideSinkProvider` (handles side connections)
- Router-based configuration will use port connections instead of the current exchange-point linking mechanism
- The router system will maintain a connection table mapping source ports to sink ports
- All router providers will connect through the new `PacketRouter` class instead of forming loops
- Flow control and packet distribution will be managed by the router rather than by loop mechanics

## Proposed New .eon File Syntax (Router-Based)

### Current Loop-Based Syntax:
```
machine sdl.app:
    driver context:
        sdl.context

    chain program:
        loop ogl.fbo:
            ogl.customer
            sdl.fbo.standalone:
                filepath = "shaders/toys/simple/simple_single/stage0.glsl"
```

### Proposed Router-Based Syntax:
```
machine sdl.app:
    driver context:
        sdl.context

    net program:
        # Atom definitions with IDs
        customer0: ogl.customer
        shader0:
            type: sdl.fbo.standalone
            filepath = "shaders/toys/simple/simple_single/stage0.glsl"

        # Connection definitions
        connections:
            customer0:0 -> shader0:0    # Connect port 0 of customer0 to port 0 of shader0
```

### More Complex Example:
```
machine sdl.app:
    driver context:
        sdl.context

    net program:
        state event.register

        # Atom definitions
        buffer_customer: ogl.customer
        buffer_shader:
            type: sdl.ogl.fbo.side
            shader.frag.path = "shaders/toys/simple/simple_double/stage1.glsl"
        screen_customer: ogl.customer
        screen_shader:
            type: sdl.fbo
            shader.frag.path = "shaders/toys/simple/simple_double/stage0.glsl"
            recv.data = false

        # Explicit connections
        connections:
            buffer_customer:0 -> buffer_shader:0
            buffer_shader:0 -> screen_shader:0  # Data flows from buffer to screen
            screen_customer:0 -> screen_shader:1 # Additional input to screen shader
```

### Key Improvements in the New Syntax:
- No need for ".out" and ".in" qualifiers since there's no other option
- No need for [] brackets; simple colon notation like "atom:port" is sufficient
- No need for "atoms:" directory; atoms can be defined directly in the net
- "chain" keyword could be renamed to "net" or "subnet" to reflect the network nature
- All connections are explicitly defined for clarity
- Atoms don't need to form closed loops, allowing for more flexible topologies
- Cross-network connections could potentially be supported with notation like "net1.atom:port -> net2.atom:port"

## Potential Challenges
- Ensuring all existing functionality is preserved during conversion
- Maintaining performance characteristics of the system
- Managing the scope of changes across multiple packages