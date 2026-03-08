from geometry import vec3
from math import sin, cos

state = {}

MAX_SPEED = 8.0
ACCEL = 10.0
DECEL = 12.0
TURN_RATE = 2.2  # radians/sec when steering held

KEY_W = 87
KEY_A = 65
KEY_S = 83
KEY_D = 68


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
    pass


def _get(key, default):
    v = state[key]
    if v == None:
        return default
    return v


def _set_scale(obj, sx, sy, sz):
    if obj == None:
        return
    obj.scale = (sx, sy, sz)


def _set_pos(obj, x, y, z):
    if obj == None:
        return
    obj.x = x
    obj.y = y
    obj.z = z


def _set_rot(obj, yaw):
    if obj == None:
        return
    obj.rotation = (yaw, 0.0, 0.0)


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
    print("drive.py: on_load")
    _find_objects()


def on_start():
    print("drive.py: on_start")
    _find_objects()
    if state["car_body"] == None:
        print("drive.py: missing car_body")
    if state["ground"] == None:
        print("drive.py: missing ground")
    if state["goal"] == None:
        print("drive.py: missing goal")
    state["car_x"] = 0.0
    state["car_z"] = 5.0
    state["car_heading"] = 0.0
    state["car_speed"] = 0.0
    state["lap"] = 0
    state["next_checkpoint"] = 0
    state["steer"] = 0.0
    state["frame_count"] = 0
    state["printed_setup"] = 0

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
    if state["printed_setup"] == 0:
        state["printed_setup"] = 1
        if state["car_body"] != None:
            print("drive.py: car_body xyz")
            print(state["car_body"].x)
            print(state["car_body"].y)
            print(state["car_body"].z)
        if state["ground"] != None:
            print("drive.py: ground xyz")
            print(state["ground"].x)
            print(state["ground"].y)
            print(state["ground"].z)


def _near_checkpoint(cx, cz, ix, iz):
    return abs(cx - ix) < 1.5 and abs(cz - iz) < 1.5


def on_frame(dt):
    fc = _get("frame_count", 0) + 1
    state["frame_count"] = fc
    if fc == 1:
        print("drive.py: on_frame")
    if state["car_body"] == None:
        _find_objects()
    if state["car_body"] == None:
        return

    car_x = _get("car_x", 0.0)
    car_z = _get("car_z", 0.0)
    heading = _get("car_heading", 0.0)
    speed = _get("car_speed", 0.0)
    lap = _get("lap", 0)
    next_checkpoint = _get("next_checkpoint", 0)
    steer = 0.0
    if input.isKeyDown(KEY_A):
        steer = -1.0
    elif input.isKeyDown(KEY_D):
        steer = 1.0
    state["steer"] = steer
    heading = heading + steer * TURN_RATE * dt

    dx = -sin(heading)
    dz = cos(heading)
    if input.isKeyDown(KEY_W):
        speed = min(MAX_SPEED, speed + ACCEL * dt)
    if input.isKeyDown(KEY_S):
        speed = max(-MAX_SPEED * 0.5, speed - DECEL * dt)

    if not input.isKeyDown(KEY_W) and not input.isKeyDown(KEY_S):
        if speed > 0.0:
            speed = max(0.0, speed - DECEL * dt)
        elif speed < 0.0:
            speed = min(0.0, speed + DECEL * dt)

    car_x = car_x + dx * speed * dt
    car_z = car_z + dz * speed * dt

    _set_pos(state["car_body"], car_x, 0.5, car_z)
    _set_rot(state["car_body"], heading)

    def _rot2d(x, z, ang):
        c = cos(ang)
        s = sin(ang)
        return [x * c - z * s, x * s + z * c]

    v = _rot2d(-0.8, 1.0, heading)
    flx = v[0]
    flz = v[1]
    v = _rot2d(0.8, 1.0, heading)
    frx = v[0]
    frz = v[1]
    v = _rot2d(-0.8, -1.0, heading)
    rlx = v[0]
    rlz = v[1]
    v = _rot2d(0.8, -1.0, heading)
    rrx = v[0]
    rrz = v[1]
    _set_pos(state["wheel_fl"], car_x + flx, 0.2, car_z + flz)
    _set_pos(state["wheel_fr"], car_x + frx, 0.2, car_z + frz)
    _set_pos(state["wheel_rl"], car_x + rlx, 0.2, car_z + rlz)
    _set_pos(state["wheel_rr"], car_x + rrx, 0.2, car_z + rrz)

    _set_rot(state["wheel_fl"], heading)
    _set_rot(state["wheel_fr"], heading)
    _set_rot(state["wheel_rl"], heading)
    _set_rot(state["wheel_rr"], heading)

    _set_scale(state["checkpoint1"], 1.0, 1.0, 1.0)
    _set_scale(state["checkpoint2"], 1.0, 1.0, 1.0)
    _set_scale(state["goal"], 1.0, 1.0, 1.0)

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
    state["car_heading"] = heading
    state["car_speed"] = speed
    state["lap"] = lap
    state["next_checkpoint"] = next_checkpoint
