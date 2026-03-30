# Test: Complete kitchen_tentacle_guard pattern

def proximity(a, b):
    return 0

def cutscene(n, fn, x):
    pass

def stop_actor(a):
    pass

def say_line_actor(a, s, b, c):
    pass

def walk_to(a, x, y):
    pass

def do_anim(a, b, c):
    pass

main_actor = {}
obj_back_door = {}
purp_tentacle = {}
selected_actor = {}
_kitchen_tentacle_stop = None

def kitchen_tentacle_guard():
    """Kitchen room - tentacle guard script"""
    while True:
        if (proximity(main_actor, obj_back_door) < 40
            and not purp_tentacle["alerting"]
            and purp_tentacle["in_room"] == selected_actor["in_room"]):
            purp_tentacle["alerting"] = True
            purp_tentacle["stopped_player"] = True
            cutscene(
                2,
                _kitchen_tentacle_stop,
                None
            )
        break_time(10)

def rm_kitchen_enter(me):
    pass

print("Test passed!")
