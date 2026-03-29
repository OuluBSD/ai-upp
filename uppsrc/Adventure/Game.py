# Adventure Game - Python version
# Converted from ESC scripting language
# Original scumm-8 copyright (c) 2017 Liquidream
# C++ conversion copyright (c) 2022 Seppo Pakonen
# Python conversion using ByteVM

# [token count history]
# 7004 (5206 is engine!) - leaving 1188 tokens spare
# ... (see original for full history)

def reset():
    # [game flags]
    global enable_diag_squeeze, verbs, verb_default
    global ui_cursorspr, ui_uparrowspr, ui_dnarrowspr, ui_cursor_cols
    
    enable_diag_squeeze = False  # allow squeeze through diag gap?

    # game verbs (used in room definitions and ui)
    verbs = [
        { "name": "open",    "text": "open" },
        { "name": "close",   "text": "close" },
        { "name": "give",    "text": "give" },
        { "name": "pickup",  "text": "pick-up" },
        { "name": "lookat",  "text": "look-at" },
        { "name": "talkto",  "text": "talk-to" },
        { "name": "push",    "text": "push" },
        { "name": "pull",    "text": "pull" },
        { "name": "use",     "text": "use" }
    ]

    # index of the verb to use when clicking items in inventory (e.g. look-at)
    verb_default = 4

    ui_cursorspr = 224  # default cursor sprite
    ui_uparrowspr = 208  # default up arrow sprite
    ui_dnarrowspr = 240  # default down arrow sprite

    # default cols to use when animating cursor
    ui_cursor_cols = [7, 12, 13, 13, 12, 7]


def reset_ui():
    global verb_maincol, verb_hovcol, verb_shadcol, verb_defcol
    
    verb_maincol = 12   # main color (lt blue)
    verb_hovcol = 7     # hover color (white)
    verb_shadcol = 1    # shadow (dk blue)
    verb_defcol = 10    # default action (yellow)


def startup_script():
    global selected_actor, main_actor
    
    # set ui colors
    reset_ui()

    # set which room to start the game in
    # (e.g. could be a "pseudo" room for title screen!)
    change_room(rm_title, 1)  # iris fade
    # change_room(rm_mi_title, 1)  # iris fade

    # set initial inventory (if applicable)
    # pickup_obj(obj_switch_tent, main_actor)
    # pickup_obj(obj_switch_player, purp_tentacle)

    # pickup_obj(obj_bucket, main_actor)
    # obj_bucket.state = "state_closed"

    # set which actor the player controls by default
    selected_actor = main_actor

    # init actor
    # put_at(selected_actor, 100, 48, rm_kitchen)
    # put_at(selected_actor, 60, 48, rm_hall)
    # put_at(selected_actor, 16, 48, rm_computer)
    # put_at(selected_actor, 110, 38, rm_garden)
    # put_at(selected_actor, 110, 38, rm_library)

    # make camera follow player
    # (setting now, will be re-instated after cutscene)
    # camera_follow(selected_actor)

    # room_curr = rm_title
    # room_curr = rm_kitchen
    # room_curr = rm_hall
    # room_curr = rm_computer
    # room_curr = rm_garden
    # room_curr = rm_library


def main():
    global obj_switch_tent, obj_switch_player, obj_duck
    global rm_title_fn0, rm_title_fn1, rm_title
    global obj_outside_stairs, obj_rail_left, obj_rail_right
    global obj_front_door, obj_bucket, rm_outside
    global obj_front_door_inside, obj_clock, obj_pendulum
    
    reset()

    obj_switch_tent = {
        "name": "purple tentacle",
        "state": "state_here",
        "state_here": 170,
        "trans_col": 15,
        "x": 1,
        "y": 1,
        "w": 1,
        "h": 1,

        "verbs": {
            "use": lambda me: (
                set_selected_actor(purp_tentacle),
                camera_follow(purp_tentacle)
            )[-1]
        }
    }

    obj_switch_player = {
        "name": "humanoid",
        "state": "state_here",
        "state_here": 209,
        "trans_col": 11,
        "x": 1,
        "y": 1,
        "w": 1,
        "h": 1,

        "verbs": {
            "use": lambda me: (
                set_selected_actor(main_actor),
                camera_follow(main_actor)
            )[-1]
        }
    }

    obj_duck = {
        "name": "rubber duck",
        "state": "state_here",
        "state_here": 142,
        "trans_col": 12,
        "x": 1,
        "y": 1,
        "w": 1,
        "h": 1,
        "classes": ["class_pickupable"],

        "verbs": {
            "pickup": lambda me: pickup_obj(me)
        }
    }

    # Rooms
    def rm_title_fn0_impl(me):
        if not me.gameover:
            # print_line("return of the...", 64, 40, 8, 1, True, 32, False)
            # for x in range(1, 8):
            #     s = "  scumm"
            #     s = s[0:x]
            #     print_line(s, 55, 45, 11, 1, True, 32, True)
            change_room(rm_outside, 1)  # iris fade
        else:
            # win game
            print_line("congratulations!:you've completed the game!", 64, 45, 8, 1, True, 8, False)
            fades(1, 1)  # fade out
            while True:
                break_time(10)

    rm_title_fn0 = rm_title_fn0_impl

    def rm_title_fn1_impl(me):
        if not rm_title.gameover:
            rm_outside.done_intro = True
            selected_actor = main_actor
            put_at(selected_actor, 30, 55, rm_outside)
            camera_follow(selected_actor)

    rm_title_fn1 = rm_title_fn1_impl

    rm_title = {
        "map": [0, 16],
        "objects": [],
        "enter": lambda me: cutscene(
            3,  # no verbs & no follow
            rm_title_fn0,
            rm_title_fn1
        ),
        "exit": lambda: None  # todo: "anything here?"
    }

    # ground floor, outside (front):
    obj_outside_stairs = {
        "x": 1,
        "y": 1,
        "classes": ["class_untouchable"],

        "draw": lambda me: (
            set_trans_col(8, True),
            map_draw(56, 23, 136, 60, 6, 1)
        )
    }

    obj_rail_left = {
        "state": "state_here",
        "x": 80,
        "y": 24,
        "w": 1,
        "h": 2,
        "state_here": 47,
        "trans_col": 8,
        "repeat_x": 8,
        "classes": ["class_untouchable"]
    }

    obj_rail_right = {
        "state": "state_here",
        "x": 176,
        "y": 24,
        "w": 1,
        "h": 2,
        "state_here": 47,
        "trans_col": 8,
        "repeat_x": 8,
        "classes": ["class_untouchable"]
    }

    obj_front_door = {
        "name": "front door",
        "state": "state_closed",
        "x": 152,
        "y": 8,
        "w": 1,
        "h": 3,
        "state_closed": 78,
        "flip_x": True,
        "classes": ["class_openable", "class_door"],
        "use_dir": "face_back",

        "init": lambda me: setattr(me, 'target_door', obj_front_door_inside)
    }

    obj_bucket = {
        "name": "bucket",
        "state": "state_open",
        "x": 208,
        "y": 48,
        "w": 1,
        "h": 1,
        "state_closed": 143,
        "state_open": 159,
        "trans_col": 15,
        "use_with": True,
        "classes": ["class_pickupable"],

        "verbs": {
            "lookat": lambda me: say_line("it's an old bucket"),
            "pickup": lambda me: pickup_obj(me),
            "use": lambda me, noun2: use_bucket(me, noun2)
        }
    }

    def use_bucket_impl(me, noun2):
        if noun2 == obj_fire and me.state == "state_closed":
            put_at(obj_fire, 0, 0, rm_void)
            put_at(obj_key, 88, 32, rm_library)
            me.state = "state_open"
            say_line("the fire's out now")
        elif noun2 == obj_pool:
            say_line("let's fill this up...")
            me.state = "state_closed"
            me.name = "full bucket"
            say_line("that's better!")

    use_bucket = use_bucket_impl

    # rm_outside room definition
    def rm_outside_enter_impl(me):
        # initialise game in first room entry...
        if not me.done_intro:
            # don't do this again
            me.done_intro = True

            # set which actor the player controls by default
            selected_actor = main_actor

            # init actor
            put_at(selected_actor, 30, 55, rm_outside)

            # make camera follow player
            # (setting now, will be re-instated after cutscene)
            camera_follow(selected_actor)

            # do cutscene
            def cutscene_fn(_):
                camera_at(144)
                camera_pan_to(selected_actor)
                wait_for_camera()
                say_line("wow! look at that old house:i wonder if anyone's home...")

            cutscene(1, cutscene_fn, None)  # no verbs

    rm_outside = {
        "map": [0, 24, 31, 31],
        "autodepth_scale": [0.75, 1],
        "objects": [
            obj_outside_stairs,
            obj_rail_left,
            obj_rail_right,
            obj_front_door,
            obj_bucket
        ],
        "enter": rm_outside_enter_impl,
        "exit": lambda me: None  # todo: "anything here?"
    }

    # hall
    obj_front_door_inside = {
        "name": "front door",
        "state": "state_closed",
        "x": 8,
        "y": 16,
        "z": 1,
        "w": 1,
        "h": 4,
        "state_closed": 79,
        "classes": ["class_door"],
        "use_pos": "pos_right",
        "use_dir": "face_left",

        "init": lambda me: setattr(me, 'target_door', obj_front_door)
    }

    obj_clock = {
        "name": "clock",
        "x": 32,
        "y": 0,
        "w": 2,
        "h": 5,
        "z": 2,

        "draw": lambda me: (
            set_trans_col(8, True),
            map_draw(56, 16, 32, 16, 2, 6)
        ),
        "verbs": {
            "lookat": lambda me: say_line("wow. that is impressive...:must've taken ages to code that!")
        }
    }

    obj_pendulum = {
        "x": 40,
        "y": 20,
        "w": 1,
        "z": 1,
        # ... (rest of pendulum definition)
    }

    # Continue with rest of game definitions...
    # This is a partial conversion - full file would continue here


# Run the main function when loaded
if __name__ == "__main__":
    main()
