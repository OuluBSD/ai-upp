from geometry import vec3

state = {}

move_speed = 6.0
shoot_cooldown = 0.2
turn_cooldown = 0.15

KEY_W = 87
KEY_A = 65
KEY_S = 83
KEY_D = 68
KEY_SPACE = 32


def _find_objects():
    state["gun"] = stage.find("gun")
    state["target"] = stage.find("target")
    state["ground"] = stage.find("ground")
    state["house1"] = stage.find("house1")
    state["house2"] = stage.find("house2")
    state["house3"] = stage.find("house3")
    state["house4"] = stage.find("house4")
    state["house5"] = stage.find("house5")
    state["bullet0"] = stage.find("bullet0")
    state["bullet1"] = stage.find("bullet1")
    state["bullet2"] = stage.find("bullet2")
    state["bullet3"] = stage.find("bullet3")


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


def _set_cam(x, y, z, yaw):
    if camera == None:
        return
    camera.x = x
    camera.y = y
    camera.z = z
    camera.rotation = (yaw, 0.0, 0.0)


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


def on_load():
    _find_objects()


def on_start():
    _find_objects()
    state["player_x"] = 0.0
    state["player_z"] = 8.0
    state["dir_idx"] = 2
    state["shoot_timer"] = 0.0
    state["turn_timer"] = 0.0

    _set_scale(state["ground"], 40.0, 0.1, 40.0)
    _set_pos(state["ground"], 0.0, -0.6, 0.0)

    _set_scale(state["gun"], 0.4, 0.2, 1.2)
    _set_scale(state["target"], 1.0, 1.0, 1.0)
    _set_pos(state["target"], 0.0, 0.5, -10.0)

    _set_scale(state["house1"], 3.0, 2.0, 3.0)
    _set_scale(state["house2"], 3.0, 2.0, 3.0)
    _set_scale(state["house3"], 3.0, 2.0, 3.0)
    _set_scale(state["house4"], 3.0, 2.0, 3.0)
    _set_scale(state["house5"], 3.0, 2.0, 3.0)

    _set_pos(state["house1"], -12.0, 1.0, -4.0)
    _set_pos(state["house2"], -6.0, 1.0, 4.0)
    _set_pos(state["house3"], 0.0, 1.0, -4.0)
    _set_pos(state["house4"], 6.0, 1.0, 4.0)
    _set_pos(state["house5"], 12.0, 1.0, -4.0)

    state["bullet0_active"] = False
    state["bullet1_active"] = False
    state["bullet2_active"] = False
    state["bullet3_active"] = False
    state["bullet0_x"] = 0.0
    state["bullet1_x"] = 0.0
    state["bullet2_x"] = 0.0
    state["bullet3_x"] = 0.0
    state["bullet0_z"] = 0.0
    state["bullet1_z"] = 0.0
    state["bullet2_z"] = 0.0
    state["bullet3_z"] = 0.0
    state["bullet0_dx"] = 0.0
    state["bullet1_dx"] = 0.0
    state["bullet2_dx"] = 0.0
    state["bullet3_dx"] = 0.0
    state["bullet0_dz"] = 0.0
    state["bullet1_dz"] = 0.0
    state["bullet2_dz"] = 0.0
    state["bullet3_dz"] = 0.0

    _set_scale(state["bullet0"], 0.2, 0.2, 0.6)
    _set_scale(state["bullet1"], 0.2, 0.2, 0.6)
    _set_scale(state["bullet2"], 0.2, 0.2, 0.6)
    _set_scale(state["bullet3"], 0.2, 0.2, 0.6)

    b0 = state["bullet0"]
    if b0 != None:
        b0.visible = False
    b1 = state["bullet1"]
    if b1 != None:
        b1.visible = False
    b2 = state["bullet2"]
    if b2 != None:
        b2.visible = False
    b3 = state["bullet3"]
    if b3 != None:
        b3.visible = False


def _spawn_bullet():
    dx = _dir_x(state["dir_idx"])
    dz = _dir_z(state["dir_idx"])
    if (not state["bullet0_active"]) and state["bullet0"] != None:
        state["bullet0_active"] = True
        state["bullet0_x"] = state["player_x"] + dx * 1.2
        state["bullet0_z"] = state["player_z"] + dz * 1.2
        state["bullet0_dx"] = dx
        state["bullet0_dz"] = dz
        b0 = state["bullet0"]
        if b0 != None:
            b0.visible = True
        return
    if (not state["bullet1_active"]) and state["bullet1"] != None:
        state["bullet1_active"] = True
        state["bullet1_x"] = state["player_x"] + dx * 1.2
        state["bullet1_z"] = state["player_z"] + dz * 1.2
        state["bullet1_dx"] = dx
        state["bullet1_dz"] = dz
        b1 = state["bullet1"]
        if b1 != None:
            b1.visible = True
        return
    if (not state["bullet2_active"]) and state["bullet2"] != None:
        state["bullet2_active"] = True
        state["bullet2_x"] = state["player_x"] + dx * 1.2
        state["bullet2_z"] = state["player_z"] + dz * 1.2
        state["bullet2_dx"] = dx
        state["bullet2_dz"] = dz
        b2 = state["bullet2"]
        if b2 != None:
            b2.visible = True
        return
    if (not state["bullet3_active"]) and state["bullet3"] != None:
        state["bullet3_active"] = True
        state["bullet3_x"] = state["player_x"] + dx * 1.2
        state["bullet3_z"] = state["player_z"] + dz * 1.2
        state["bullet3_dx"] = dx
        state["bullet3_dz"] = dz
        b3 = state["bullet3"]
        if b3 != None:
            b3.visible = True
        return


def _update_bullet(idx, dt):
    if idx == 0:
        if not state["bullet0_active"]:
            return
        state["bullet0_x"] = state["bullet0_x"] + state["bullet0_dx"] * 12.0 * dt
        state["bullet0_z"] = state["bullet0_z"] + state["bullet0_dz"] * 12.0 * dt
        if state["bullet0"] != None:
            _set_pos(state["bullet0"], state["bullet0_x"], 0.4, state["bullet0_z"])
        if abs(state["bullet0_x"]) > 30.0 or abs(state["bullet0_z"]) > 30.0:
            state["bullet0_active"] = False
            b0 = state["bullet0"]
            if b0 != None:
                b0.visible = False
            return
        if state["target"] != None:
            if abs(state["bullet0_x"] - state["target"].x) < 0.8 and abs(state["bullet0_z"] - state["target"].z) < 0.8:
                print("Target hit")
                exec.exit()
        return
    if idx == 1:
        if not state["bullet1_active"]:
            return
        state["bullet1_x"] = state["bullet1_x"] + state["bullet1_dx"] * 12.0 * dt
        state["bullet1_z"] = state["bullet1_z"] + state["bullet1_dz"] * 12.0 * dt
        if state["bullet1"] != None:
            _set_pos(state["bullet1"], state["bullet1_x"], 0.4, state["bullet1_z"])
        if abs(state["bullet1_x"]) > 30.0 or abs(state["bullet1_z"]) > 30.0:
            state["bullet1_active"] = False
            b1 = state["bullet1"]
            if b1 != None:
                b1.visible = False
            return
        if state["target"] != None:
            if abs(state["bullet1_x"] - state["target"].x) < 0.8 and abs(state["bullet1_z"] - state["target"].z) < 0.8:
                print("Target hit")
                exec.exit()
        return
    if idx == 2:
        if not state["bullet2_active"]:
            return
        state["bullet2_x"] = state["bullet2_x"] + state["bullet2_dx"] * 12.0 * dt
        state["bullet2_z"] = state["bullet2_z"] + state["bullet2_dz"] * 12.0 * dt
        if state["bullet2"] != None:
            _set_pos(state["bullet2"], state["bullet2_x"], 0.4, state["bullet2_z"])
        if abs(state["bullet2_x"]) > 30.0 or abs(state["bullet2_z"]) > 30.0:
            state["bullet2_active"] = False
            b2 = state["bullet2"]
            if b2 != None:
                b2.visible = False
            return
        if state["target"] != None:
            if abs(state["bullet2_x"] - state["target"].x) < 0.8 and abs(state["bullet2_z"] - state["target"].z) < 0.8:
                print("Target hit")
                exec.exit()
        return
    if not state["bullet3_active"]:
        return
    state["bullet3_x"] = state["bullet3_x"] + state["bullet3_dx"] * 12.0 * dt
    state["bullet3_z"] = state["bullet3_z"] + state["bullet3_dz"] * 12.0 * dt
    if state["bullet3"] != None:
        _set_pos(state["bullet3"], state["bullet3_x"], 0.4, state["bullet3_z"])
    if abs(state["bullet3_x"]) > 30.0 or abs(state["bullet3_z"]) > 30.0:
        state["bullet3_active"] = False
        b3 = state["bullet3"]
        if b3 != None:
            b3.visible = False
        return
    if state["target"] != None:
        if abs(state["bullet3_x"] - state["target"].x) < 0.8 and abs(state["bullet3_z"] - state["target"].z) < 0.8:
            print("Target hit")
            exec.exit()


def on_frame(dt):
    if state["gun"] == None:
        _find_objects()
    if state["gun"] == None:
        return

    if state["turn_timer"] > 0.0:
        state["turn_timer"] = state["turn_timer"] - dt
    if state["turn_timer"] <= 0.0:
        if input.isKeyDown(KEY_A):
            state["dir_idx"] = (state["dir_idx"] - 1) % 4
            state["turn_timer"] = turn_cooldown
        elif input.isKeyDown(KEY_D):
            state["dir_idx"] = (state["dir_idx"] + 1) % 4
            state["turn_timer"] = turn_cooldown

    dx = _dir_x(state["dir_idx"])
    dz = _dir_z(state["dir_idx"])
    if input.isKeyDown(KEY_W):
        state["player_x"] = state["player_x"] + dx * move_speed * dt
        state["player_z"] = state["player_z"] + dz * move_speed * dt
    if input.isKeyDown(KEY_S):
        state["player_x"] = state["player_x"] - dx * move_speed * dt
        state["player_z"] = state["player_z"] - dz * move_speed * dt

    _set_pos(state["gun"], state["player_x"] + dx * 0.8, 0.5, state["player_z"] + dz * 0.8)
    yaw = _dir_rot(state["dir_idx"])
    _set_rot(state["gun"], yaw)
    _set_cam(state["player_x"], 1.6, state["player_z"], yaw)

    if state["shoot_timer"] > 0.0:
        state["shoot_timer"] = state["shoot_timer"] - dt
    if state["shoot_timer"] <= 0.0 and input.wasKeyPressed(KEY_SPACE):
        state["shoot_timer"] = shoot_cooldown
        _spawn_bullet()

    _update_bullet(0, dt)
    _update_bullet(1, dt)
    _update_bullet(2, dt)
    _update_bullet(3, dt)
