#ifndef _Maestro_ConversionPlanner_h_
#define _Maestro_ConversionPlanner_h_

class ConversionPlanner {
public:
	static WorkGraph GeneratePlan(const MaestroInventory& source, const MaestroInventory& target, ConversionMemory& memory);
	static void      AddAutoCheckpoints(WorkGraph& wg, ConversionMemory& memory);
};

#endif
