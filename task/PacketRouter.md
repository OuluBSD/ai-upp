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

## Potential Challenges
- Ensuring all existing functionality is preserved during conversion
- Maintaining performance characteristics of the system
- Managing the scope of changes across multiple packages