#ifndef RAINBOWGAME_PLATFORMSERVICES_H
#define RAINBOWGAME_PLATFORMSERVICES_H

#include <Core/Core.h>

using namespace Upp;

class PlatformServices {
public:
    PlatformServices();
    ~PlatformServices();
    
    class AdBridge {
    public:
        AdBridge();
        ~AdBridge();
        
        void Initialize(const String& config);
        void ShowInterstitial();
        void ShowRewardedAd();
        bool IsAdReady(const String& adType);
        
    private:
        String config;
        bool initialized;
    };
    
    AdBridge* GetAdBridge() { return &adBridge; }
    
    // Other platform services could go here:
    // - Analytics
    // - Cloud save
    // - Social features
    // etc.
    
private:
    AdBridge adBridge;
};

#endif