// Additional test for stdsrc CtrlLib functionality (Splitter, ScrollBar, SliderCtrl)

#include "CtrlLib/CtrlLib.h"

using namespace Upp;

// This would be added to the CtrlLibTest.cpp file
void TestAdditionalCtrls() {
    // Test Splitter functionality
    {
        Splitter splitter;
        
        // Test initial state
        ASSERT(!splitter.Vert().GetClassName() == std::string("Splitter"));
        ASSERT(splitter.GetPos() == 100);  // Default position
        
        // Test position setting
        splitter.SetPos(50);
        ASSERT(splitter.GetPos() == 50);
        
        // Test range setting
        splitter.SetPosRange(10, 200);
        ASSERT(splitter.GetPos() == 50);  // Still 50, within new range
        splitter.SetPos(5);
        ASSERT(splitter.GetPos() == 10);  // Clamped to min
        splitter.SetPos(250);
        ASSERT(splitter.GetPos() == 200);  // Clamped to max
        
        // Test orientation
        Splitter vertSplitter;
        vertSplitter.Vert();
        // Orientation is stored but not easily tested without specific getter
        
        RLOG("Splitter tests passed!");
    }
    
    // Test ScrollBar functionality
    {
        ScrollBar scrollbar;
        
        // Test initial state
        ASSERT(scrollbar.GetPage() == 10);
        ASSERT(scrollbar.GetTotal() == 100);
        ASSERT(scrollbar.GetLine() == 1);
        ASSERT(scrollbar.GetPos() == 0);
        
        // Test range setting
        scrollbar.SetTotal(200);
        ASSERT(scrollbar.GetTotal() == 200);
        
        scrollbar.SetPage(20);
        ASSERT(scrollbar.GetPage() == 20);
        
        scrollbar.SetLine(5);
        ASSERT(scrollbar.GetLine() == 5);
        
        // Test position setting with clamping
        scrollbar.SetPos(185);  // This should be valid: 185 + 20 = 205 <= 200? No, max pos is total-page = 180
        ASSERT(scrollbar.GetPos() == 180);  // Clamped to max possible (total-page)
        
        scrollbar.SetPos(-10);
        ASSERT(scrollbar.GetPos() == 0);  // Clamped to minimum
        
        // Test orientation
        ScrollBar vertScrollbar;
        vertScrollbar.Vert();
        // Orientation is stored but not easily tested without specific getter
        
        RLOG("ScrollBar tests passed!");
    }
    
    // Test SliderCtrl functionality
    {
        SliderCtrl slider;
        
        // Test initial state
        ASSERT(slider.GetMin() == 0);
        ASSERT(slider.GetMax() == 100);
        ASSERT(slider.GetValue() == 50);  // Default value
        ASSERT(slider.GetTickFreq() == 10);
        
        // Test range setting
        slider.SetRange(10, 90);
        ASSERT(slider.GetMin() == 10);
        ASSERT(slider.GetMax() == 90);
        ASSERT(slider.GetValue() == 50);  // Within new range
        ASSERT(slider.GetMax() - slider.GetMin() == 80);
        
        // Test value setting with clamping
        slider.SetValue(5);
        ASSERT(slider.GetValue() == 10);  // Clamped to minimum
        
        slider.SetValue(95);
        ASSERT(slider.GetValue() == 90);  // Clamped to maximum
        
        slider.SetValue(75);
        ASSERT(slider.GetValue() == 75);  // Valid value
        
        // Test tick frequency
        slider.SetTickFreq(5);
        ASSERT(slider.GetTickFreq() == 5);
        
        // Test orientation
        SliderCtrl vertSlider;
        vertSlider.Vert();
        // Orientation is stored but not easily tested without specific getter
        
        RLOG("SliderCtrl tests passed!");
    }
    
    RLOG("All additional CtrlLib tests passed!");
}

CONSOLE_APP_MAIN {
    TestAdditionalCtrls();
}