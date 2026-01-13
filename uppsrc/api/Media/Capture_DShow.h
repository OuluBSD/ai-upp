#ifndef _api_Media_Capture_DShow_h_
#define _api_Media_Capture_DShow_h_

#if HAVE_DIRECTSHOW

#include <Core/Core.h>

#ifdef flagWIN32
#define CY win32_CY_
#define FAR win32_FAR_
#endif

#include <dshow.h>

#ifdef flagWIN32
#undef CY
#undef FAR
#endif

NAMESPACE_UPP

class DShowCaptureDevice {
public:
    DShowCaptureDevice();
    ~DShowCaptureDevice();

    bool Open(int deviceIndex);
    void Close();
    bool IsOpened() const;
    bool GetFrame(Image& img);
    
    static int GetDeviceCount();
    static String GetDeviceName(int index);
};

END_UPP_NAMESPACE

#endif // HAVE_DIRECTSHOW

#endif