#include "DShow_inc.h"
#include "Capture_DShow.h"

#if HAVE_DIRECTSHOW

NAMESPACE_UPP

DShowCaptureDevice::DShowCaptureDevice() {
    // Constructor
}

DShowCaptureDevice::~DShowCaptureDevice() {
    Close();
}

bool DShowCaptureDevice::Open(int deviceIndex) {
    // Stub
    return false;
}

void DShowCaptureDevice::Close() {
    // Stub
}

bool DShowCaptureDevice::GetFrame(Image& img) {
    // Stub
    return false;
}

int DShowCaptureDevice::GetDeviceCount() {
    // Stub
    return 0;
}

String DShowCaptureDevice::GetDeviceName(int index) {
    // Stub
    return "Stub Device";
}

END_UPP_NAMESPACE

#endif