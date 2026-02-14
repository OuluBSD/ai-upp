from geometry import vec3
from math import sin, cos

state = {}

MAX_SPEED = 8.0
ACCEL = 10.0
DECEL = 12.0
TURN_RATE = 2.2  # radians/sec

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
    obj.rotation = (0.0, yaw, 0.0)


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
    print("drive_softphys.py: on_load")
    _find_objects()


def on_start():
    print("drive_softphys.py: on_start")
    _find_objects()

    if state["car_body"] == None:
        print("drive_softphys.py: missing car_body")
    if state["ground"] == None:
        print("drive_softphys.py: missing ground")

    state["car_heading"] = 0.0
    state["car_speed"] = 0.0
    state["lap"] = 0
    state["next_checkpoint"] = 0
    state["frame_count"] = 0
    state["ai_enabled"] = True
    state["route"] = ()
    state["route_pos"] = 0
    state["checkpoint_pts"] = ()

    _set_scale(state["ground"], 25.0, 0.2, 25.0)
    _set_pos(state["ground"], 0.0, -0.6, 0.0)

    _set_scale(state["car_body"], 1.5, 0.6, 3.0)
    _set_pos(state["car_body"], 0.0, 0.5, 5.0)

    _set_scale(state["wheel_fl"], 0.4, 0.4, 0.4)
    _set_scale(state["wheel_fr"], 0.4, 0.4, 0.4)
    _set_scale(state["wheel_rl"], 0.4, 0.4, 0.4)
    _set_scale(state["wheel_rr"], 0.4, 0.4, 0.4)

    _set_scale(state["checkpoint1"], 1.0, 1.0, 1.0)
    _set_scale(state["checkpoint2"], 1.0, 1.0, 1.0)
    _set_scale(state["goal"], 1.0, 1.0, 1.0)

    _set_pos(state["checkpoint1"], _checkpoint_x(0), 0.6, _checkpoint_z(0))
    _set_pos(state["checkpoint2"], _checkpoint_x(1), 0.6, _checkpoint_z(1))
    _set_pos(state["goal"], _checkpoint_x(2), 0.6, _checkpoint_z(2))

    state["checkpoint_pts"] = (
        (_checkpoint_x(0), 0.0, _checkpoint_z(0)),
        (_checkpoint_x(1), 0.0, _checkpoint_z(1)),
        (_checkpoint_x(2), 0.0, _checkpoint_z(2)),
    )
    route = driver_ai.compute_route(state["checkpoint_pts"], 0, 2, 20.0)
    if route == None or len(route) == 0:
        route = (0, 1, 2)
    state["route"] = route
    state["route_pos"] = 0

    state["world"] = physics.create_world()
    state["space"] = physics.create_space(state["world"])
    state["car_body_rb"] = physics.create_box(state["space"], (1.5, 0.6, 3.0), 1.0, (0.0, 0.5, 5.0))
    physics.bind(state["car_body"], state["car_body_rb"], True)


def _near_checkpoint(cx, cz, ix, iz):
    return abs(cx - ix) < 1.5 and abs(cz - iz) < 1.5


def on_frame(dt):
    fc = _get("frame_count", 0) + 1
    state["frame_count"] = fc
    if fc == 1:
        print("drive_softphys.py: on_frame")

    if state["car_body"] == None:
        _find_objects()
    if state["car_body"] == None:
        return

    heading = _get("car_heading", 0.0)
    speed = _get("car_speed", 0.0)
    lap = _get("lap", 0)
    next_checkpoint = _get("next_checkpoint", 0)

    manual = input.isKeyDown(KEY_W) or input.isKeyDown(KEY_S) or input.isKeyDown(KEY_A) or input.isKeyDown(KEY_D)
    ai_enabled = _get("ai_enabled", True)

    steer = 0.0
    ai_action = 0
    if ai_enabled and not manual:
        route = _get("route", ())
        route_pos = _get("route_pos", 0)
        if route_pos >= len(route):
            route_pos = 0
        if len(route) > 0:
            target_idx = route[route_pos]
        else:
            target_idx = next_checkpoint
        pts = _get("checkpoint_pts", ())
        target = (_checkpoint_x(target_idx), 0.0, _checkpoint_z(target_idx))
        if len(pts) > target_idx:
            target = pts[target_idx]
        car_body_rb = state["car_body_rb"]
        car_pos = (0.0, 0.0, 0.0)
        if car_body_rb != None:
            pos = car_body_rb.position
            car_pos = (pos[0], 0.0, pos[2])
        ai_action = driver_ai.plan_action(car_pos, heading, target, 1.5, 0.35)
        if ai_action == -1:
            steer = -1.0
        elif ai_action == 1:
            steer = 1.0
        else:
            steer = 0.0
    else:
        if input.isKeyDown(KEY_A):
            steer = -1.0
        elif input.isKeyDown(KEY_D):
            steer = 1.0

    heading = heading + steer * TURN_RATE * dt

    if ai_enabled and not manual:
        if ai_action == 2:
            if speed > 0.0:
                speed = max(0.0, speed - DECEL * dt)
        else:
            speed = min(MAX_SPEED, speed + ACCEL * dt)
    else:
        if input.isKeyDown(KEY_W):
            speed = min(MAX_SPEED, speed + ACCEL * dt)
        if input.isKeyDown(KEY_S):
            speed = max(-MAX_SPEED * 0.5, speed - DECEL * dt)
        if not input.isKeyDown(KEY_W) and not input.isKeyDown(KEY_S):
            if speed > 0.0:
                speed = max(0.0, speed - DECEL * dt)
            elif speed < 0.0:
                speed = min(0.0, speed + DECEL * dt)

    dx = -sin(heading)
    dz = cos(heading)

    car_body_rb = state["car_body_rb"]
    if car_body_rb != None:
        car_body_rb.velocity = (dx * speed, 0.0, dz * speed)
        car_body_rb.rotation = (0.0, heading, 0.0)
        pos = car_body_rb.position
        car_body_rb.position = (pos[0], 0.5, pos[2])

    physics.step(state["world"], dt)

    car_x = 0.0
    car_z = 0.0
    if car_body_rb != None:
        pos = car_body_rb.position
        car_x = pos[0]
        car_z = pos[2]

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

    if ai_enabled and not manual:
        route = _get("route", ())
        route_pos = _get("route_pos", 0)
        if route_pos < len(route):
            target_idx = route[route_pos]
            tx = _checkpoint_x(target_idx)
            tz = _checkpoint_z(target_idx)
            if _near_checkpoint(car_x, car_z, tx, tz):
                route_pos = route_pos + 1
                if route_pos >= len(route):
                    route_pos = 0
                state["route_pos"] = route_pos

    state["car_heading"] = heading
    state["car_speed"] = speed
    state["lap"] = lap
    state["next_checkpoint"] = next_checkpoint
