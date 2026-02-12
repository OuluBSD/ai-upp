from geometry import vec3

state = {}

SPEED = 6.0
TURN_COOLDOWN = 0.25


def _find_objects():
    state["car_body"] = stage.find("car_body")
    state["wheel_fl"] = stage.find("wheel_fl")
    state["wheel_fr"] = stage.find("wheel_fr")
    state["wheel_rl"] = stage.find("wheel_rl")
    state["wheel_rr"] = stage.find("wheel_rr")
    state["ground"] = stage.find("ground")
    state["checkpoint1"] = stage.find("checkpoint1")
    state["checkpoint2"] = stage.find("checkpoint2")
    state["goal"] = stage.find("goal")


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


def _set_rot(obj, yaw):
    if obj == None:
        return
    obj.rotation = vec3(0.0, yaw, 0.0)


def _dir_x(idx):
    if idx == 0:
        return 0.0
    if idx == 1:
        return 1.0
    if idx == 2:
        return 0.0
    return -1.0


def _dir_z(idx):
    if idx == 0:
        return 1.0
    if idx == 1:
        return 0.0
    if idx == 2:
        return -1.0
    return 0.0


def _dir_rot(idx):
    if idx == 0:
        return 0.0
    if idx == 1:
        return 1.5708
    if idx == 2:
        return 3.1416
    return 4.7124


def _checkpoint_x(i):
    if i == 0:
        return 0.0
    if i == 1:
        return 10.0
    if i == 2:
        return 0.0
    return 0.0


def _checkpoint_z(i):
    if i == 0:
        return 18.0
    if i == 1:
        return 0.0
    if i == 2:
        return -18.0
    return 18.0


def on_load():
    _find_objects()


def on_start():
    _find_objects()
    state["car_x"] = 0.0
    state["car_z"] = 5.0
    state["dir_idx"] = 0
    state["lap"] = 0
    state["next_checkpoint"] = 0
    state["turn_timer"] = 0.0

    _set_scale(state["ground"], 25.0, 0.2, 25.0)
    _set_pos(state["ground"], 0.0, -0.6, 0.0)

    _set_scale(state["car_body"], 1.5, 0.6, 3.0)
    _set_pos(state["car_body"], _get("car_x", 0.0), 0.5, _get("car_z", 0.0))

    _set_scale(state["wheel_fl"], 0.4, 0.4, 0.4)
    _set_scale(state["wheel_fr"], 0.4, 0.4, 0.4)
    _set_scale(state["wheel_rl"], 0.4, 0.4, 0.4)
    _set_scale(state["wheel_rr"], 0.4, 0.4, 0.4)

    _set_pos(state["wheel_fl"], _get("car_x", 0.0) - 0.8, 0.2, _get("car_z", 0.0) + 1.0)
    _set_pos(state["wheel_fr"], _get("car_x", 0.0) + 0.8, 0.2, _get("car_z", 0.0) + 1.0)
    _set_pos(state["wheel_rl"], _get("car_x", 0.0) - 0.8, 0.2, _get("car_z", 0.0) - 1.0)
    _set_pos(state["wheel_rr"], _get("car_x", 0.0) + 0.8, 0.2, _get("car_z", 0.0) - 1.0)

    _set_scale(state["checkpoint1"], 1.0, 1.0, 1.0)
    _set_scale(state["checkpoint2"], 1.0, 1.0, 1.0)
    _set_scale(state["goal"], 1.0, 1.0, 1.0)

    _set_pos(state["checkpoint1"], _checkpoint_x(0), 0.6, _checkpoint_z(0))
    _set_pos(state["checkpoint2"], _checkpoint_x(1), 0.6, _checkpoint_z(1))
    _set_pos(state["goal"], _checkpoint_x(2), 0.6, _checkpoint_z(2))


def _near_checkpoint(cx, cz, ix, iz):
    return abs(cx - ix) < 1.5 and abs(cz - iz) < 1.5


def on_frame(dt):
    if state["car_body"] == None:
        _find_objects()
    if state["car_body"] == None:
        return

    car_x = _get("car_x", 0.0)
    car_z = _get("car_z", 0.0)
    dir_idx = _get("dir_idx", 0)
    lap = _get("lap", 0)
    next_checkpoint = _get("next_checkpoint", 0)
    turn_timer = _get("turn_timer", 0.0)

    if turn_timer > 0.0:
        turn_timer = turn_timer - dt
    if turn_timer <= 0.0:
        if input.isKeyDown(ord('A')):
            dir_idx = (dir_idx - 1) % 4
            turn_timer = TURN_COOLDOWN
        elif input.isKeyDown(ord('D')):
            dir_idx = (dir_idx + 1) % 4
            turn_timer = TURN_COOLDOWN

    dx = _dir_x(dir_idx)
    dz = _dir_z(dir_idx)
    if input.isKeyDown(ord('W')):
        car_x = car_x + dx * SPEED * dt
        car_z = car_z + dz * SPEED * dt
    if input.isKeyDown(ord('S')):
        car_x = car_x - dx * SPEED * dt
        car_z = car_z - dz * SPEED * dt

    _set_pos(state["car_body"], car_x, 0.5, car_z)
    _set_rot(state["car_body"], _dir_rot(dir_idx))

    _set_pos(state["wheel_fl"], car_x - 0.8, 0.2, car_z + 1.0)
    _set_pos(state["wheel_fr"], car_x + 0.8, 0.2, car_z + 1.0)
    _set_pos(state["wheel_rl"], car_x - 0.8, 0.2, car_z - 1.0)
    _set_pos(state["wheel_rr"], car_x + 0.8, 0.2, car_z - 1.0)

    _set_rot(state["wheel_fl"], _dir_rot(dir_idx))
    _set_rot(state["wheel_fr"], _dir_rot(dir_idx))
    _set_rot(state["wheel_rl"], _dir_rot(dir_idx))
    _set_rot(state["wheel_rr"], _dir_rot(dir_idx))

    cx = _checkpoint_x(next_checkpoint)
    cz = _checkpoint_z(next_checkpoint)
    if _near_checkpoint(car_x, car_z, cx, cz):
        next_checkpoint = next_checkpoint + 1
        if next_checkpoint >= 3:
            next_checkpoint = 0
            lap = lap + 1
            print("Lap", lap)
            if lap >= 3:
                exec.exit()

    state["car_x"] = car_x
    state["car_z"] = car_z
    state["dir_idx"] = dir_idx
    state["lap"] = lap
    state["next_checkpoint"] = next_checkpoint
    state["turn_timer"] = turn_timer
