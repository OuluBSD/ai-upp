#ifndef _AnimEdit_EntityPreviewCtrl_h_
#define _AnimEdit_EntityPreviewCtrl_h_

#include <CtrlLib/CtrlLib.h>
#include <AnimEditLib/AnimCore.h>

using namespace Upp;

class EntityPreviewCtrl : public Ctrl {
public:
    typedef EntityPreviewCtrl CLASSNAME;

    EntityPreviewCtrl();
    virtual ~EntityPreviewCtrl();

    void SetProject(const AnimationProject* project);
    void SetEntity(const Entity* entity);
    void SetAnimation(const Animation* animation);

private:
    const AnimationProject* project;
    const Entity* entity;
    const Animation* animation;
    int current_frame_index;
    bool is_playing;
    bool loop_enabled;
    TimeCallback animation_timer;

    void StartAnimation();
    void StopAnimation();
    void UpdateAnimation();
    void OnTimer();

    virtual void Paint(Draw& w) override;
    virtual void Layout() override;
};

#endif