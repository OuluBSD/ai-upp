#include <SoftHMD/SoftHMD.h>
#include <Core/Core.h>
#include <iostream>

using namespace Upp;

CONSOLE_APP_MAIN
{
	HMD::Context* ctx = HMD::CreateContext();
	if(!ctx) {
		std::cerr << "Failed to create HMD context" << std::endl;
		return;
	}

	int n = HMD::ProbeContext(ctx);
	if(n <= 0) {
		std::cout << "No devices found." << std::endl;
		HMD::DestroyContext(ctx);
		return;
	}

	HMD::Device* hmd = NULL;
	for(int i = 0; i < n; i++) {
		int flags = 0;
		HMD::GetListInt(ctx, i, HMD::HMD_DEVICE_FLAGS, &flags);
		if(!(flags & HMD::HMD_DEVICE_FLAGS_NULL_DEVICE)) {
			hmd = HMD::OpenListDevice(ctx, i);
			if(hmd) {
				std::cout << "Opened device: " << HMD::GetListString(ctx, i, HMD::HMD_PRODUCT) << std::endl;
				break;
			}
			else {
				std::cerr << "Failed to open device " << i << ": " << HMD::GetContextError(ctx) << std::endl;
			}
		}
	}

	if(!hmd) {
		std::cerr << "Could not open any hardware device." << std::endl;
		HMD::DestroyContext(ctx);
		return;
	}

	std::cout << "Starting data loop (Ctrl+C to stop)..." << std::endl;

	while(!Thread::IsShutdownThreads()) {
		HMD::UpdateContext(ctx);
		
		float rot[4], pos[3], acc[3], gyro[3];
		
HMD::GetDeviceFloat(hmd, HMD::HMD_ROTATION_QUAT, rot);
		HMD::GetDeviceFloat(hmd, HMD::HMD_POSITION_VECTOR, pos);
		HMD::GetDeviceFloat(hmd, HMD::HMD_ACCELEROMETER_VECTOR, acc);
		HMD::GetDeviceFloat(hmd, HMD::HMD_GYROSCOPE_VECTOR, gyro);
		
		printf("\rROT: %6.3f %6.3f %6.3f %6.3f | ACC: %6.3f %6.3f %6.3f | GYRO: %6.3f %6.3f %6.3f",
			rot[0], rot[1], rot[2], rot[3],
			acc[0], acc[1], acc[2],
			gyro[0], gyro[1], gyro[2]);
		fflush(stdout);
		
		Sleep(10); // ~100Hz display update
	}

	HMD::CloseDevice(hmd);
	HMD::DestroyContext(ctx);
	std::cout << "\nCleaned up." << std::endl;
}
