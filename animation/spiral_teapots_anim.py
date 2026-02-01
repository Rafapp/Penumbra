import os
import math

# === Animation ===
frames = 48
teapots = 8

# Motion along +Y (toward camera at y = -60 looking to y = 0)
y_far  = 26.0   # behind back wall (invisible)
y_near = -36.0  # past camera (invisible)

phase_offset = 0.0   # [0,1) shifts the whole loop (use to line up framing)
eps = 1e-3           # continuation step to avoid re-hitting same light triangles (scene units)

# === IO ===
windows = True
mac = False
scene_path = ""
output_dir = ""

if windows:
    scene_path = r"C:\Users\rpadi\Documents\Dev\PenumbraDev\anim\template\cornell.pbrt"
    output_dir = r"C:\Users\rpadi\Documents\Dev\PenumbraDev\anim\gen\cornell"
elif mac:
    scene_path = "/Users/rafa/Documents/Dev/PenumbraDev/Penumbra/resources/scenes/cornell.pbrt"
    output_dir = "/Users/rafa/Documents/Dev/PenumbraDev/Penumbra/resources/scenes/cornell_spiral"

os.makedirs(output_dir, exist_ok=True)

# Spiral order (front view):
# 3 4 5
# 2   6
# 1 8 7
comment_to_rank = {
    "# Bottom left": 1,
    "# Middle left": 2,
    "# Top left": 3,
    "# Top middle": 4,
    "# Top right": 5,
    "# Middle right": 6,
    "# Bottom right": 7,
    "# Bottom middle": 8,
}

span = y_far - y_near  # > 0

def y_for_rank(rank: int, t: float) -> float:
    phase = phase_offset + (rank - 1) / teapots
    u = (t + phase) % 1.0
    return y_far - u * span

with open(scene_path, "r") as f:
    template_lines = f.readlines()

for i in range(frames):
    t = i / frames
    y_by_rank = {r: y_for_rank(r, t) for r in range(1, teapots + 1)}

    active_rank = None
    out_lines = []

    for line in template_lines:
        stripped = line.strip()

        if stripped in comment_to_rank:
            active_rank = comment_to_rank[stripped]
            out_lines.append(line)
            continue

        # Rewrite the *first* Translate after a teapot comment; then clear active_rank
        if active_rank is not None and stripped.startswith("Translate") and not stripped.startswith("#"):
            parts = line.split()
            x = float(parts[1])
            z = float(parts[3])
            y = y_by_rank[active_rank]
            out_lines.append(f"  Translate {x:.6f} {y:.6f} {z:.6f}\n")
            active_rank = None
            continue

        out_lines.append(line)

    out_path = os.path.join(output_dir, f"{i+1}.pbrt")
    with open(out_path, "w") as f:
        f.writelines(out_lines)

print(f"Generated {frames} frames in {output_dir}")
