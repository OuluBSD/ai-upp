from geometry import vec3

state = {}

forward_speed = 6.0
side_speed = 4.0


def _find_objects():
    state["rocket"] = stage.find("rocket")
    state["obstacle0"] = stage.find("obstacle0")
    state["obstacle1"] = stage.find("obstacle1")
    state["obstacle2"] = stage.find("obstacle2")
    state["obstacle3"] = stage.find("obstacle3")
    state["ground"] = stage.find("ground")


def _get(key, default):
    v = state[key]
    if v == None:
        return default
    return v


def _set_scale(obj, sx, sy, sz):
    if obj == None:
        return
    obj.scale = vec3(sx, sy, sz)


def _set_pos(obj, x, y, z):
    if obj == None:
        return
    obj.x = x
    obj.y = y
    obj.z = z


def _slot_x(slot):
    if slot == 0:
        return -2.0
    if slot == 1:
        return 0.0
    return 2.0


def _slot_y(slot):
    if slot == 0:
        return 0.5
    if slot == 1:
        return 1.5
    return 0.5


def _advance_slot(slot):
    if slot == 0:
        return 1
    if slot == 1:
        return 2
    return 0


def on_load():
    _find_objects()


def on_start():
    _find_objects()
    state["rocket_x"] = 0.0
    state["rocket_y"] = 1.0
    state["rocket_z"] = 0.0

    state["obs0_z"] = -10.0
    state["obs1_z"] = -25.0
    state["obs2_z"] = -40.0
    state["obs3_z"] = -55.0

    state["obs0_slot"] = 0
    state["obs1_slot"] = 1
    state["obs2_slot"] = 2
    state["obs3_slot"] = 0

    _set_scale(state["rocket"], 0.6, 0.6, 1.4)
    _set_scale(state["obstacle0"], 2.0, 2.0, 2.0)
    _set_scale(state["obstacle1"], 2.0, 2.0, 2.0)
    _set_scale(state["obstacle2"], 2.0, 2.0, 2.0)
    _set_scale(state["obstacle3"], 2.0, 2.0, 2.0)
    _set_scale(state["ground"], 30.0, 0.1, 200.0)
    _set_pos(state["ground"], 0.0, -1.0, -80.0)

    state["obs0_x"] = _slot_x(state["obs0_slot"])
    state["obs0_y"] = _slot_y(state["obs0_slot"])
    state["obs1_x"] = _slot_x(state["obs1_slot"])
    state["obs1_y"] = _slot_y(state["obs1_slot"])
    state["obs2_x"] = _slot_x(state["obs2_slot"])
    state["obs2_y"] = _slot_y(state["obs2_slot"])
    state["obs3_x"] = _slot_x(state["obs3_slot"])
    state["obs3_y"] = _slot_y(state["obs3_slot"])

    _set_pos(state["rocket"], state["rocket_x"], state["rocket_y"], state["rocket_z"])
    _set_pos(state["obstacle0"], state["obs0_x"], state["obs0_y"], state["obs0_z"])
    _set_pos(state["obstacle1"], state["obs1_x"], state["obs1_y"], state["obs1_z"])
    _set_pos(state["obstacle2"], state["obs2_x"], state["obs2_y"], state["obs2_z"])
    _set_pos(state["obstacle3"], state["obs3_x"], state["obs3_y"], state["obs3_z"])


def _collides(rx, ry, rz, ox, oy, oz):
    if abs(rx - ox) > 1.2:
        return False
    if abs(ry - oy) > 1.2:
        return False
    if abs(rz - oz) > 1.2:
        return False
    return True


def _update_obstacle(idx, dt):
    if idx == 0:
        state["obs0_z"] = state["obs0_z"] + forward_speed * dt
        if state["obs0_z"] > 5.0:
            state["obs0_z"] = -60.0
            state["obs0_slot"] = _advance_slot(state["obs0_slot"])
            state["obs0_x"] = _slot_x(state["obs0_slot"])
            state["obs0_y"] = _slot_y(state["obs0_slot"])
        _set_pos(state["obstacle0"], state["obs0_x"], state["obs0_y"], state["obs0_z"])
        return
    if idx == 1:
        state["obs1_z"] = state["obs1_z"] + forward_speed * dt
        if state["obs1_z"] > 5.0:
            state["obs1_z"] = -60.0
            state["obs1_slot"] = _advance_slot(state["obs1_slot"])
            state["obs1_x"] = _slot_x(state["obs1_slot"])
            state["obs1_y"] = _slot_y(state["obs1_slot"])
        _set_pos(state["obstacle1"], state["obs1_x"], state["obs1_y"], state["obs1_z"])
        return
    if idx == 2:
        state["obs2_z"] = state["obs2_z"] + forward_speed * dt
        if state["obs2_z"] > 5.0:
            state["obs2_z"] = -60.0
            state["obs2_slot"] = _advance_slot(state["obs2_slot"])
            state["obs2_x"] = _slot_x(state["obs2_slot"])
            state["obs2_y"] = _slot_y(state["obs2_slot"])
        _set_pos(state["obstacle2"], state["obs2_x"], state["obs2_y"], state["obs2_z"])
        return

    state["obs3_z"] = state["obs3_z"] + forward_speed * dt
    if state["obs3_z"] > 5.0:
        state["obs3_z"] = -60.0
        state["obs3_slot"] = _advance_slot(state["obs3_slot"])
        state["obs3_x"] = _slot_x(state["obs3_slot"])
        state["obs3_y"] = _slot_y(state["obs3_slot"])
    _set_pos(state["obstacle3"], state["obs3_x"], state["obs3_y"], state["obs3_z"])


def on_frame(dt):
    if state["rocket"] == None:
        _find_objects()
    if state["rocket"] == None:
        return

    if input.isKeyDown(ord('A')):
        state["rocket_x"] = state["rocket_x"] - side_speed * dt
    if input.isKeyDown(ord('D')):
        state["rocket_x"] = state["rocket_x"] + side_speed * dt
    if input.isKeyDown(ord('W')):
        state["rocket_y"] = state["rocket_y"] + side_speed * dt
    if input.isKeyDown(ord('S')):
        state["rocket_y"] = state["rocket_y"] - side_speed * dt

    _set_pos(state["rocket"], state["rocket_x"], state["rocket_y"], state["rocket_z"])

    _update_obstacle(0, dt)
    _update_obstacle(1, dt)
    _update_obstacle(2, dt)
    _update_obstacle(3, dt)

    if _collides(state["rocket_x"], state["rocket_y"], state["rocket_z"], state["obs0_x"], state["obs0_y"], state["obs0_z"]):
        print("Crashed")
        exec.exit()
    if _collides(state["rocket_x"], state["rocket_y"], state["rocket_z"], state["obs1_x"], state["obs1_y"], state["obs1_z"]):
        print("Crashed")
        exec.exit()
    if _collides(state["rocket_x"], state["rocket_y"], state["rocket_z"], state["obs2_x"], state["obs2_y"], state["obs2_z"]):
        print("Crashed")
        exec.exit()
    if _collides(state["rocket_x"], state["rocket_y"], state["rocket_z"], state["obs3_x"], state["obs3_y"], state["obs3_z"]):
        print("Crashed")
        exec.exit()
