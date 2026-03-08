#!/usr/bin/env python3
import math
import sys

if len(sys.argv) < 2:
    print("Usage: compare_render_math.py <dump-file>")
    sys.exit(1)

path = sys.argv[1]

lines = [l.strip() for l in open(path, "r", encoding="utf-8") if l.strip()]

def parse_vec3(tokens, i):
    return [float(tokens[i]), float(tokens[i+1]), float(tokens[i+2])], i+3

def parse_vec2(tokens, i):
    return [float(tokens[i]), float(tokens[i+1])], i+2

idx = 0
size = None
world = []
proj = []
view = []
cam_pos = None
cam_orient = None

while idx < len(lines):
    parts = lines[idx].split()
    if parts[0] == "size":
        size = (int(parts[1]), int(parts[2]))
        idx += 1
    elif parts[0] == "cam_pos":
        cam_pos = list(map(float, parts[1:4]))
        idx += 1
    elif parts[0] == "cam_orient":
        cam_orient = list(map(float, parts[1:5]))
        idx += 1
    elif parts[0] == "world":
        world = []
        for _ in range(4):
            idx += 1
            world.append(list(map(float, lines[idx].split())))
        idx += 1
    elif parts[0] == "proj":
        proj = []
        for _ in range(4):
            idx += 1
            proj.append(list(map(float, lines[idx].split())))
        idx += 1
    elif parts[0] == "view":
        view = []
        for _ in range(4):
            idx += 1
            view.append(list(map(float, lines[idx].split())))
        idx += 1
    elif parts[0] == "lines":
        line_count = int(parts[1])
        idx += 1
        break
    else:
        idx += 1

if not world or not proj:
    print("Missing matrices in dump")
    sys.exit(1)


def mat_vec_mul(m, v):
    # U++ matrices are effectively column-major in multiplication
    x, y, z, w = v
    out = [
        m[0][0]*x + m[1][0]*y + m[2][0]*z + m[3][0]*w,
        m[0][1]*x + m[1][1]*y + m[2][1]*z + m[3][1]*w,
        m[0][2]*x + m[1][2]*y + m[2][2]*z + m[3][2]*w,
        m[0][3]*x + m[1][3]*y + m[2][3]*z + m[3][3]*w,
    ]
    return out


def ndc_from_proj_world(proj, world, p):
    v = mat_vec_mul(world, [p[0], p[1], p[2], 1.0])
    v = mat_vec_mul(proj, v)
    if v[3] == 0:
        return [float("nan"), float("nan"), float("nan")]
    return [v[0]/v[3], v[1]/v[3], v[2]/v[3]]


def clip_line_ndc(a, b):
    dx = b[0] - a[0]
    dy = b[1] - a[1]
    p = [-dx, dx, -dy, dy]
    q = [a[0] + 1.0, 1.0 - a[0], a[1] + 1.0, 1.0 - a[1]]
    u1 = 0.0
    u2 = 1.0
    for i in range(4):
        if p[i] == 0.0:
            if q[i] < 0.0:
                return False, a, b
        else:
            t = q[i] / p[i]
            if p[i] < 0.0:
                u1 = max(u1, t)
            else:
                u2 = min(u2, t)
            if u1 > u2:
                return False, a, b
    a0 = a[:]
    a = [a0[0] + dx * u1, a0[1] + dy * u1]
    b = [a0[0] + dx * u2, a0[1] + dy * u2]
    return True, a, b


max_ndc_err = 0.0
max_clip_err = 0.0
line_idx = 0
for i in range(line_count):
    tokens = lines[idx + i].split()
    if tokens[0] != "line":
        continue
    t = tokens[1:]
    a_world, pos = parse_vec3(t, 0)
    b_world, pos = parse_vec3(t, pos)
    a_world_clipped, pos = parse_vec3(t, pos)
    b_world_clipped, pos = parse_vec3(t, pos)
    a_cam, pos = parse_vec3(t, pos)
    b_cam, pos = parse_vec3(t, pos)
    a_ndc, pos = parse_vec3(t, pos)
    b_ndc, pos = parse_vec3(t, pos)
    a_clip, pos = parse_vec2(t, pos)
    b_clip, pos = parse_vec2(t, pos)
    culled = int(t[pos]); clipped = int(t[pos+1])

    a_ndc_py = ndc_from_proj_world(proj, world, a_world_clipped)
    b_ndc_py = ndc_from_proj_world(proj, world, b_world_clipped)

    err_a = sum(abs(a_ndc_py[j] - a_ndc[j]) for j in range(3))
    err_b = sum(abs(b_ndc_py[j] - b_ndc[j]) for j in range(3))
    max_ndc_err = max(max_ndc_err, err_a, err_b)

    ok, a_clip_py, b_clip_py = clip_line_ndc([a_ndc_py[0], a_ndc_py[1]], [b_ndc_py[0], b_ndc_py[1]])
    if ok and clipped:
        err_c = sum(abs(a_clip_py[j] - a_clip[j]) for j in range(2))
        err_d = sum(abs(b_clip_py[j] - b_clip[j]) for j in range(2))
        max_clip_err = max(max_clip_err, err_c, err_d)

    if err_a > 1e-3 or err_b > 1e-3:
        print(f"Line {line_idx}: NDC mismatch a={err_a:.6f} b={err_b:.6f}")
        print("  a_world_clipped", a_world_clipped, "a_ndc", a_ndc, "a_ndc_py", a_ndc_py)
        print("  b_world_clipped", b_world_clipped, "b_ndc", b_ndc, "b_ndc_py", b_ndc_py)
        break
    line_idx += 1

print("max_ndc_err", max_ndc_err)
print("max_clip_err", max_clip_err)
