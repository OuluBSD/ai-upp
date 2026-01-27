#include <SoftHMD/SoftHMD.h>
#include <Core/Core.h>
#include <iostream>

using namespace Upp;

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT | LOG_FILE);

	HMD::Context* ctx = HMD::CreateContext();
	if(!ctx) {
		std::cerr << "Failed to create HMD context" << std::endl;
		return;
	}

	std::cout << "HMD Context created successfully." << std::endl;

	// Accessing internal context to verify drivers
	// We need to include Internal.h which is already included by SoftHMD.h 
	// but let's see if we can see the drivers.
	std::cout << "Number of registered drivers: " << ctx->num_drivers << std::endl;
	for(int i = 0; i < ctx->num_drivers; i++) {
		// Driver struct doesn't have a name in SoftHMD, but we can see the count
		std::cout << "Driver " << i << " is registered." << std::endl;
	}

	int num_devices = HMD::ProbeContext(ctx);
	std::cout << "Number of devices found: " << num_devices << std::endl;

	for(int i = 0; i < num_devices; i++) {
		const char* vendor = HMD::GetListString(ctx, i, HMD::HMD_VENDOR);
		const char* product = HMD::GetListString(ctx, i, HMD::HMD_PRODUCT);
		const char* path = HMD::GetListString(ctx, i, HMD::HMD_PATH);
		std::cout << "Device " << i << ": " << vendor << " " << product << " (" << path << ")" << std::endl;
	}

	HMD::DestroyContext(ctx);
	std::cout << "HMD Context destroyed." << std::endl;
}
