#ifndef _AnimEdit_EntityPropertiesCtrl_h_
#define _AnimEdit_EntityPropertiesCtrl_h_

#include <CtrlLib/CtrlLib.h>
#include <AnimEditLib/AnimCore.h>
#include "EntityPreviewCtrl.h"

using namespace Upp;

class EntityPropertiesCtrl : public Ctrl {
public:
    typedef EntityPropertiesCtrl CLASSNAME;

    EntityPropertiesCtrl();
    virtual ~EntityPropertiesCtrl();

    void SetProject(const AnimationProject* project);
    void SetEntity(const Entity* entity);
    void SetChangeCallback(std::function<void()> callback);

    // Get the current entity values (for saving back to project)
    Entity GetEntity() const;

private:
    const AnimationProject* project;
    Entity* current_entity;  // Not owned, just a reference to the one in project
    std::function<void()> change_callback;
    
    // Controls for entity properties
    EditField id_field;
    EditField name_field;
    Option type_option;
    ArrayCtrl animation_slots_ctrl;
    Option animation_dropdown;  // For selecting available animations
    EditField slot_name_field;  // For naming the slot
    EditField property_name_field;
    EditField property_value_field;
    Button add_property_btn;
    Button remove_property_btn;
    ArrayCtrl properties_ctrl;
    Button add_slot_btn;
    Button remove_slot_btn;
    
    // Validation feedback
    StaticRect validation_status;  // Shows validation status
    
    // Preview control
    EntityPreviewCtrl preview_ctrl;

    void UpdateControls();
    void OnEntityChanged();
    void OnSlotChanged();
    void OnPropertyChanged();
    void OnAddSlot();
    void OnRemoveSlot();
    void OnAddProperty();
    void OnRemoveProperty();
    void ValidateEntity();

    virtual void Paint(Draw& w) override;
    virtual void Layout() override;
};

#endif