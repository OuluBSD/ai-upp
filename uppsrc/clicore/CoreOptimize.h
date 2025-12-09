#ifndef _clicore_CoreOptimize_h_
#define _clicore_CoreOptimize_h_

#include "clicore.h"

class CoreIde;

class CoreOptimize : public Moveable<CoreOptimize> {
public:
    struct LoopConfig : Moveable<LoopConfig> {
        int max_iterations = 10;
        bool stop_on_worse = true;   // If metrics degrade, stop immediately
        bool stop_on_converge = true; // If delta < threshold, stop
        double converge_threshold = 0.01;
    };

    struct LoopIteration : Moveable<LoopIteration> {
        int index;
        Value before_stats;
        Value after_stats;
        Value changes;     // e.g., edit counts, renamed symbols, include removals
        double score_delta;
    };

    struct LoopResult : Moveable<LoopResult> {
        bool success;
        Vector<LoopIteration> iterations;
        String reason;      // why terminated
    };

    CoreOptimize();

    LoopResult RunOptimizationLoop(
        const String& package,
        const LoopConfig& config,
        CoreIde& ide,
        String& error
    );

private:
    double ComputeScore(const Value& telemetry);
    Value DetectChanges(const CoreIde& ide_before,
                        const CoreIde& ide_after);
};

#endif