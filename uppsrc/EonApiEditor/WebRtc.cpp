#include "EonApiEditor.h"

NAMESPACE_UPP

void InterfaceBuilder::AddWebRtc() {
    Package("WebRtc", "Wr");
    SetColor(0, 200, 100);
    Dependency("ParallelLib");
    Dependency("ports/webrtc", "BUILTIN_WEBRTC");
    Library("webrtc", "WEBRTC");
    
    Interface("PeerConnection");
    Interface("DataChannel");
    Interface("MediaStream");
    Interface("MediaStreamTrack");
    Interface("RtpSender");
    Interface("RtpReceiver");
    Interface("RtpTransceiver");
    Interface("IceCandidate");
    Interface("SessionDescription");
    
    Vendor("LibWebRtc", "BUILTIN_WEBRTC|LIBWEBRTC");
    Vendor("WebRtcNative", "WEBRTC_NATIVE");
}

END_UPP_NAMESPACE