#include "EntityPropertiesCtrl.h"

EntityPropertiesCtrl::EntityPropertiesCtrl() {
    project = nullptr;
    current_entity = nullptr;
    
    // Create the layout
    CtrlLayout(*this);
    
    // Set up type options
    type_option.Add("character");
    type_option.Add("environment");
    type_option.Add("item");
    type_option.Add("other");
    type_option <<= 0;  // Default to character
    
    // Set up animation slots control
    animation_slots_ctrl.AddColumn("Slot Name", 100);
    animation_slots_ctrl.AddColumn("Animation ID", 150);
    animation_slots_ctrl.SetFrame(ThinInsetFrame());
    animation_slots_ctrl.NoHeader();
    
    // Set up properties control
    properties_ctrl.AddColumn("Property Name", 120);
    properties_ctrl.AddColumn("Value", 120);
    properties_ctrl.SetFrame(ThinInsetFrame());
    properties_ctrl.NoHeader();
    
    // Set up slot editing fields
    slot_name_field.SetPrompt("Slot Name");
    slot_name_field.SetTip("Name for the animation slot (e.g. Idle, Run, Attack)");
    
    // Set up animation dropdown
    animation_dropdown.Add("(No Animation)");
    animation_dropdown.SetTip("Select animation for this slot");
    
    // Set up property editing fields
    add_property_btn.SetLabel("+");
    remove_property_btn.SetLabel("-");
    add_slot_btn.SetLabel("+");
    remove_slot_btn.SetLabel("-");
    
    // Set up validation status display
    validation_status.SetFrame(ThinInsetFrame());
    validation_status.SetLabel("Validation: OK");
    
    // Set up animation playback controls
    play_btn.SetLabel("▶ Play");
    play_btn.SetTip("Play the selected animation");
    pause_btn.SetLabel("⏸ Pause");
    pause_btn.SetTip("Pause the animation");
    stop_btn.SetLabel("⏹ Stop");
    stop_btn.SetTip("Stop and reset the animation");
    loop_check.SetLabel("Loop");
    loop_check.SetTip("Loop the animation continuously");
    loop_check.Set(true);  // Loop by default
    speed_label.SetLabel("Speed: 1.0x");
    speed_slider.SetRange(1, 500).SetValue(100);  // Range 0.01x to 5.0x (value/100)
    speed_slider.SetTip("Adjust animation playback speed");
    
    // Set up blending controls
    blend_weight_label.SetLabel("Blend Weight: 1.00");
    blend_weight_slider.SetRange(0, 100).SetValue(100);  // 0.00 to 1.00 as 0-100
    blend_weight_slider.SetTip("Adjust the blend weight for the selected animation (0.0 to 1.0)");
    transition_label.SetLabel("Transition Time (s):");
    transition_time_edit.SetRange(0.0, 5.0).Set(0.0);  // 0 to 5 seconds
    transition_time_edit.SetTip("Time to transition to this animation (in seconds)");
    apply_blend_btn.SetLabel("Apply Blend");
    apply_blend_btn.SetTip("Apply the current blend settings to the selected animation slot");
    
    // Connect events
    id_field.WhenAction = [this]() { OnEntityChanged(); };
    name_field.WhenAction = [this]() { OnEntityChanged(); };
    type_option.WhenAction = [this]() { OnEntityChanged(); };
    animation_slots_ctrl.WhenLeftClick = [this]() { OnSlotChanged(); };
    properties_ctrl.WhenLeftClick = [this]() { OnPropertyChanged(); };
    add_property_btn <<= [this]() { OnAddProperty(); };
    remove_property_btn <<= [this]() { OnRemoveProperty(); };
    add_slot_btn <<= [this]() { OnAddSlot(); };
    remove_slot_btn <<= [this]() { OnRemoveSlot(); };
    animation_dropdown.WhenAction = [this]() { OnSlotChanged(); };
    slot_name_field.WhenAction = [this]() { OnSlotChanged(); };
    play_btn <<= [this]() { OnPlayClicked(); };
    pause_btn <<= [this]() { OnPauseClicked(); };
    stop_btn <<= [this]() { OnStopClicked(); };
    loop_check.WhenAction = [this]() { OnLoopToggled(); };
    speed_slider.WhenAction = [this]() { OnSpeedChanged(); };
    blend_weight_slider.WhenAction = [this]() { OnBlendWeightChanged(); };
    transition_time_edit.WhenAction = [this]() { OnTransitionTimeChanged(); };
    apply_blend_btn <<= [this]() { OnApplyBlendClicked(); };
}

EntityPropertiesCtrl::~EntityPropertiesCtrl() {
}

void EntityPropertiesCtrl::SetProject(const AnimationProject* proj) {
    project = proj;
    // Update controls if we have an entity selected
    if (current_entity) {
        UpdateControls();
    }
}

void EntityPropertiesCtrl::SetEntity(const Entity* entity) {
    current_entity = const_cast<Entity*>(entity);
    UpdateControls();
}

void EntityPropertiesCtrl::SetChangeCallback(std::function<void()> callback) {
    change_callback = callback;
}

Entity EntityPropertiesCtrl::GetEntity() const {
    if (!current_entity) {
        return Entity();
    }

    // We need to update the entity with current values from controls
    Entity updated_entity = *current_entity;
    updated_entity.id = ~id_field;
    updated_entity.name = ~name_field;
    updated_entity.type = AsString(type_option.Get());

    // For animation slots and properties, we would need to populate them from the controls
    // This is a simplified implementation
    return updated_entity;
}

void EntityPropertiesCtrl::UpdateControls() {
    if (!current_entity) {
        // Clear all controls
        id_field.Clear();
        name_field.Clear();
        type_option <<= 0;
        animation_slots_ctrl.Clear();
        properties_ctrl.Clear();

        // Clear the preview
        preview_ctrl.SetProject(project);
        preview_ctrl.SetEntity(nullptr);
        preview_ctrl.SetAnimation(nullptr);

        // Clear and populate animation dropdown with available animations
        if (project) {
            animation_dropdown.Clear();
            animation_dropdown.Add("(No Animation)");
            for (const auto& anim : project->animations) {
                animation_dropdown.Add(anim.id);
            }
        }
        return;
    }

    // Update basic entity fields
    id_field = current_entity->id;
    name_field = current_entity->name;

    // Update type selection
    int type_idx = type_option.Find(current_entity->type);
    if (type_idx >= 0) {
        type_option <<= type_idx;
    } else {
        type_option <<= 0;  // Default to first option
    }

    // Clear and populate animation dropdown with available animations
    animation_dropdown.Clear();
    animation_dropdown.Add("(No Animation)");
    if (project) {
        for (const auto& anim : project->animations) {
            animation_dropdown.Add(anim.id);
        }
    }

    // Update animation slots
    animation_slots_ctrl.Clear();
    for (const auto& slot : current_entity->animation_slots) {
        animation_slots_ctrl.Add(slot.name, slot.animation_id);

        // If this is the currently selected slot in the array control,
        // update the dropdown to show the selected animation
        if (animation_slots_ctrl.GetCount() - 1 == animation_slots_ctrl.GetCursor()) {
            int anim_idx = animation_dropdown.Find(slot.animation_id);
            if (anim_idx >= 0) {
                animation_dropdown <<= anim_idx;
            } else {
                animation_dropdown <<= 0; // "(No Animation)"
            }
        }
    }

    // Update properties
    properties_ctrl.Clear();
    for (const auto& kv : current_entity->properties) {
        properties_ctrl.Add(kv.GetKey(), SConvert<ConvertTxt>::Single<ConvertTxt>(kv.GetValue()));
    }

    // Update the preview with the current entity and its first animation
    preview_ctrl.SetProject(project);
    preview_ctrl.SetEntity(current_entity);
    if (current_entity->animation_slots.GetCount() > 0) {
        const NamedAnimationSlot& slot = current_entity->animation_slots[0];
        if (project) {
            const Animation* anim = project->FindAnimation(slot.animation_id);
            preview_ctrl.SetAnimation(anim);
        }
    }

    // Validate the entity after updating controls
    ValidateEntity();
}

void EntityPropertiesCtrl::OnEntityChanged() {
    if (current_entity && change_callback) {
        // Update the current entity with values from UI
        current_entity->id = ~id_field;
        current_entity->name = ~name_field;
        current_entity->type = AsString(type_option.Get());
        change_callback();
        ValidateEntity();
    }
}

void EntityPropertiesCtrl::OnSlotChanged() {
    // Animation slot selection changed - update the selected slot's animation
    if (!current_entity) return;

    int selected_slot_idx = animation_slots_ctrl.GetCursor();
    if (selected_slot_idx >= 0 && selected_slot_idx < current_entity->animation_slots.GetCount()) {
        // Update the currently selected slot
        String selected_anim_id = AsString(animation_dropdown.Get());

        // Only update if it's different
        if (current_entity->animation_slots[selected_slot_idx].animation_id != selected_anim_id) {
            current_entity->animation_slots[selected_slot_idx].animation_id = selected_anim_id;
            if (change_callback) {
                change_callback();
            }
            ValidateEntity();
        }
        
        // Update blending controls to reflect current slot values
        const auto& slot = current_entity->animation_slots[selected_slot_idx];
        blend_weight_slider.SetPos((int)(slot.blend_params.weight * 100)); // Convert 0.0-1.0 to 0-100
        String weightStr = Format("%.2f", slot.blend_params.weight);
        blend_weight_label.SetLabel("Blend Weight: " + weightStr);
        transition_time_edit <<= slot.blend_params.transition_time;
    }
}

void EntityPropertiesCtrl::OnPropertyChanged() {
    // Property selection changed
    if (change_callback) {
        change_callback();
    }
}

void EntityPropertiesCtrl::OnAddSlot() {
    if (!current_entity) return;

    // Get the slot name from the field, default to "NewSlot" if empty
    String slot_name = ~slot_name_field;
    if (slot_name.IsEmpty()) {
        slot_name = "NewSlot";
    }

    // Add a new animation slot
    NamedAnimationSlot new_slot;
    new_slot.name = slot_name;
    new_slot.animation_id = "(No Animation)"; // Default to no animation

    current_entity->animation_slots.Add(new_slot);

    UpdateControls();
    if (change_callback) {
        change_callback();
    }
}

void EntityPropertiesCtrl::OnRemoveSlot() {
    if (!current_entity) return;

    int sel = animation_slots_ctrl.GetCursor();
    if (sel >= 0 && sel < current_entity->animation_slots.GetCount()) {
        current_entity->animation_slots.Remove(sel);
        UpdateControls();
        if (change_callback) {
            change_callback();
        }
    }
}

void EntityPropertiesCtrl::OnAddProperty() {
    if (!current_entity) return;

    // Add a new property with default values
    String name = ~property_name_field;
    if (name.IsEmpty()) {
        name = "NewProp";
    }

    // Check if a property with this name already exists to avoid duplicates
    if (current_entity->properties.Find(name)) {
        Exclamation("A property with name '" + name + "' already exists!");
        return;
    }

    // For now, we'll add a string value - in a full implementation, we'd have
    // type selection options
    current_entity->properties.Set(name, ~property_value_field);

    // Clear the input fields after adding the property
    property_name_field.Clear();
    property_value_field.Clear();

    UpdateControls();
    if (change_callback) {
        change_callback();
    }
    ValidateEntity();
}

void EntityPropertiesCtrl::OnRemoveProperty() {
    if (!current_entity) return;

    int sel = properties_ctrl.GetCursor();
    if (sel >= 0) {
        String key = properties_ctrl.Get(0, sel);
        current_entity->properties.Remove(key);
        UpdateControls();
        if (change_callback) {
            change_callback();
        }
    }
}

void EntityPropertiesCtrl::ValidateEntity() {
    if (!current_entity) {
        validation_status.SetLabel("Validation: No entity selected");
        validation_status.SetInk(Black());
        validation_status.SetPaper(LightGray());
        return;
    }

    if (!project) {
        validation_status.SetLabel("Validation: No project loaded");
        validation_status.SetInk(Black());
        validation_status.SetPaper(LightGray());
        return;
    }

    String errorMsg;
    if (ValidateEntity(*project, *current_entity, errorMsg)) {
        validation_status.SetLabel("Validation: OK");
        validation_status.SetInk(Green());
        validation_status.SetPaper(LightGreen());
    } else {
        validation_status.SetLabel("Validation: Error - " + errorMsg);
        validation_status.SetInk(Red());
        validation_status.SetPaper(LightYellow());
    }

    // Refresh the validation control
    validation_status.Refresh();
}

void EntityPropertiesCtrl::Paint(Draw& w) {
    w.DrawRect(GetSize(), White());
}

void EntityPropertiesCtrl::Layout() {
    // Layout the properties control
    Size sz = GetSize();

    // Calculate heights for each section
    int validation_height = 30;
    int preview_height = 150;  // Height for the preview area
    int top_height = sz.cy - preview_height - validation_height;

    // Position the property controls in the top area
    id_field.SetRect(10, 10, sz.cx - 20, 24);
    name_field.SetRect(10, 40, sz.cx - 20, 24);

    // Position the type option
    type_option.SetRect(10, 70, sz.cx - 20, 24);

    // Position the animation slots controls
    int slot_y = 100;
    slot_name_field.SetRect(10, slot_y, 120, 24);
    animation_dropdown.SetRect(140, slot_y, sz.cx - 150, 24);
    add_slot_btn.SetRect(sz.cx - 60, slot_y, 24, 24);
    remove_slot_btn.SetRect(sz.cx - 30, slot_y, 24, 24);

    // Position the animation slots list
    animation_slots_ctrl.SetRect(10, slot_y + 30, sz.cx - 20, 80);

    // Position the properties controls
    int prop_y = slot_y + 115;
    property_name_field.SetRect(10, prop_y, 120, 24);
    property_value_field.SetRect(140, prop_y, sz.cx - 180, 24);
    add_property_btn.SetRect(sz.cx - 40, prop_y, 24, 24);
    remove_property_btn.SetRect(sz.cx - 10, prop_y, 24, 24);

    // Position the properties list
    properties_ctrl.SetRect(10, prop_y + 30, sz.cx - 20, 80);

    // Position animation playback controls (in the properties area)
    int controls_y = prop_y + 115;
    play_btn.SetRect(10, controls_y, 60, 24);
    pause_btn.SetRect(80, controls_y, 60, 24);
    stop_btn.SetRect(150, controls_y, 60, 24);
    loop_check.SetRect(220, controls_y, 60, 24);
    speed_label.SetRect(290, controls_y, 80, 24);
    speed_slider.SetRect(370, controls_y + 4, sz.cx - 380, 16);

    // Position blending controls (below the playback controls)
    int blend_y = controls_y + 30;
    blend_weight_label.SetRect(10, blend_y, 120, 24);
    blend_weight_slider.SetRect(140, blend_y + 4, 150, 16);
    transition_label.SetRect(300, blend_y, 120, 24);
    transition_time_edit.SetRect(420, blend_y, 60, 24);
    apply_blend_btn.SetRect(490, blend_y, 100, 24);

    // Position the preview control at the bottom (above validation)
    int preview_y = sz.cy - preview_height - validation_height;
    preview_ctrl.SetRect(0, preview_y, sz.cx, preview_height);

    // Position the validation status at the very bottom
    validation_status.SetRect(0, sz.cy - validation_height, sz.cx, validation_height);
}

void EntityPropertiesCtrl::OnPlayClicked() {
    if (preview_ctrl.GetAnimation()) {
        preview_ctrl.StartAnimation();
    } else {
        Exclamation("No animation selected to play!");
    }
}

void EntityPropertiesCtrl::OnPauseClicked() {
    preview_ctrl.PauseAnimation();
}

void EntityPropertiesCtrl::OnStopClicked() {
    preview_ctrl.StopAnimation();
}

void EntityPropertiesCtrl::OnLoopToggled() {
    preview_ctrl.SetLoopEnabled(loop_check);
}

void EntityPropertiesCtrl::OnSpeedChanged() {
    double speed = speed_slider.GetValue() / 100.0;  // Convert 1-500 range to 0.01-5.0
    String speedText = Format("%.2f", speed) + "x";
    speed_label.SetLabel("Speed: " + speedText);
    preview_ctrl.SetPlaybackSpeed(speed);
    Refresh(); // Refresh the control to update the label
}

void EntityPropertiesCtrl::OnBlendWeightChanged() {
    if (!current_entity) return;
    
    int selected_slot_idx = animation_slots_ctrl.GetCursor();
    if (selected_slot_idx >= 0 && selected_slot_idx < current_entity->animation_slots.GetCount()) {
        double weight = blend_weight_slider.GetPos() / 100.0; // Convert 0-100 to 0.0-1.0
        current_entity->animation_slots[selected_slot_idx].blend_params.weight = weight;
        
        String weightStr = Format("%.2f", weight);
        blend_weight_label.SetLabel("Blend Weight: " + weightStr);
        
        if (change_callback) {
            change_callback();
        }
    }
}

void EntityPropertiesCtrl::OnTransitionTimeChanged() {
    if (!current_entity) return;
    
    int selected_slot_idx = animation_slots_ctrl.GetCursor();
    if (selected_slot_idx >= 0 && selected_slot_idx < current_entity->animation_slots.GetCount()) {
        double transition_time = ~transition_time_edit;
        current_entity->animation_slots[selected_slot_idx].blend_params.transition_time = transition_time;
        
        if (change_callback) {
            change_callback();
        }
    }
}

void EntityPropertiesCtrl::OnApplyBlendClicked() {
    if (!current_entity) return;
    
    int selected_slot_idx = animation_slots_ctrl.GetCursor();
    if (selected_slot_idx >= 0 && selected_slot_idx < current_entity->animation_slots.GetCount()) {
        // Update the slot with current UI values
        double weight = blend_weight_slider.GetPos() / 100.0; // Convert 0-100 to 0.0-1.0
        double transition_time = ~transition_time_edit;
        
        current_entity->animation_slots[selected_slot_idx].blend_params.weight = weight;
        current_entity->animation_slots[selected_slot_idx].blend_params.transition_time = transition_time;
        
        // Update the UI elements to reflect the changes
        String weightStr = Format("%.2f", weight);
        blend_weight_label.SetLabel("Blend Weight: " + weightStr);
        
        if (change_callback) {
            change_callback();
        }
        
        // Show confirmation
        PromptOK("Blend parameters applied to animation slot '" + 
                 current_entity->animation_slots[selected_slot_idx].name + "'");
    } else {
        Exclamation("Please select an animation slot first!");
    }
}