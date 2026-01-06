import os
import math

# === Camera ===
height = 20
radius = 100

# === Animation ===
frames = 96
scene_path = 'C:\\Users\\rpadi\\Documents\\Dev\\PenumbraDev\\anim\\template\\envmap.pbrt'
output_dir = os.path.expanduser('C:\\Users\\rpadi\\Documents\\Dev\\PenumbraDev\\anim\\gen\\envmap')

os.makedirs(output_dir, exist_ok=True)

for i in range(frames):
    t = i / frames
    angle = t * 2 * math.pi
    
    cam_x = math.cos(angle) * radius
    cam_y = math.sin(angle) * radius
    cam_z = height
    
    output_file = os.path.join(output_dir, f"{i+1}.pbrt")
    
    try:
        with open(scene_path, 'r') as file:
            lines = file.readlines()
        
        output_lines = []
        for line in lines:
            if line.strip().startswith('LookAt'):
                parts = line.split()
                target_x, target_y, target_z = parts[4], parts[5], parts[6]
                up_vector = f"{parts[-3]} {parts[-2]} {parts[-1]}"
                output_lines.append(f"LookAt {cam_x} {cam_y} {cam_z}  {target_x} {target_y} {target_z}  {up_vector}\n")
            else:
                output_lines.append(line)
        
        with open(output_file, 'w') as file:
            file.writelines(output_lines)
    
    except FileNotFoundError:
        print(f"Error: '{scene_path}' not found")
        break

print(f"Generated {frames} frames in {output_dir}")
