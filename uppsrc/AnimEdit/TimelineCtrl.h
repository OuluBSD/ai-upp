#ifndef _AnimEdit_TimelineCtrl_h_
#define _AnimEdit_TimelineCtrl_h_

#include <CtrlLib/CtrlLib.h>
#include <AnimEditLib/AnimCore.h>
#include <functional>

using namespace Upp;

class TimelineCtrl : public Ctrl {
public:
    typedef TimelineCtrl CLASSNAME;

    TimelineCtrl();
    virtual ~TimelineCtrl();

    void SetProject(const AnimationProject* project);
    void SetAnimation(const Animation* animation);
    void SetFrameCallback(std::function<void(const Frame*)> callback);
    void SetOnFrameModified(std::function<void()> callback);

protected:
    virtual void Paint(Draw& w);
    virtual void MouseDown(Point pos, dword button);
    virtual void MouseMove(Point pos, dword keyflags);
    virtual void LeftDown(Point pos, dword flags);
    virtual void LeftUp(Point pos, dword flags);
    virtual bool Key(dword key, int count);

private:
    const AnimationProject* project;
    const Animation* animation;
    int selected_frame_index;
    int drag_start_index;
    bool is_dragging;

    // Callbacks
    std::function<void(const Frame*)> frame_callback;
    std::function<void()> on_frame_modified_callback;

    // Layout
    int frame_width;
    int frame_height;
    int frame_spacing;

    // Methods
    int HitTest(Point pos) const; // Returns frame index or -1
    void DrawFrame(Draw& w, int index, const Rect& rc);
    void RefreshLayout();
    void UpdateScroll();
    void SelectFrame(int index);
};

#endif