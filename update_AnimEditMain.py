#!/usr/bin/env python3

# Read the current file
with open('/common/active/sblo/Dev/ai-upp/uppsrc/AnimEdit/AnimEditMain.cpp', 'r') as f:
    content = f.read()

# Split the content at the point where we want to insert
# Find the end of the sort option connection and before UpdateZoomLabel
insertion_point = "sprite_list_ctrl.SetSortType(sortType);\n    };"

new_code = '''        sprite_list_ctrl.SetSortType(sortType);
    };

    // Connect frame list control
    frame_list_ctrl.SetProject(&state.project);
    frame_list_ctrl.SetSelectCallback([this](const Frame* frame) {
        // Callback when a frame is selected in the frames list
        SetActiveFrame(frame);
        canvas_ctrl.SetFrame(frame);
        canvas_ctrl.Refresh();
    });

    // Connect animation list control
    anim_list_ctrl.SetProject(&state.project);
    anim_list_ctrl.SetSelectCallback([this](const Animation* anim) {
        // Callback when an animation is selected in the animations list
        selected_animation = anim;
        timeline_ctrl.SetAnimation(anim);
        timeline_ctrl.Refresh();
    });

    // Connect new frame button
    new_frame_btn <<= [this] {
        // Create a new frame and add it to the project
        Frame new_frame;
        new_frame.id = "frame_" + Uuid().ToString();
        new_frame.name = "New Frame";
        state.project.frames.Add(new_frame);

        // If there's a selected animation, add the new frame to it
        if (selected_animation) {
            FrameRef frame_ref;
            frame_ref.frame_id = new_frame.id;
            frame_ref.has_duration = false;
            frame_ref.duration = 0.1; // default duration
            selected_animation->frames.Add(frame_ref);
        }

        state.dirty = true;
        UpdateTitle();
        frame_list_ctrl.RefreshList();
    };

    // Connect new animation button
    new_anim_btn <<= [this] {
        // Create a new animation and add it to the project
        Animation new_anim;
        new_anim.id = "anim_" + Uuid().ToString();
        new_anim.name = "New Animation";
        new_anim.category = "default";
        new_anim.loop_type = "No Loop";
        state.project.animations.Add(new_anim);

        state.dirty = true;
        UpdateTitle();
        anim_list_ctrl.RefreshList();
    };'''

# Replace the content
content = content.replace(insertion_point, new_code)

# Write the updated content back to the file
with open('/common/active/sblo/Dev/ai-upp/uppsrc/AnimEdit/AnimEditMain.cpp', 'w') as f:
    f.write(content)

print("File updated successfully!")