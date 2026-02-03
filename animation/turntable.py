import os
import math

# === Camera ===
height = 45.0 
radius = 35.0 

# === Animation ===
frames = 96 
windows = True 
mac = False 
scene_path = ""
output_dir = ""

if(windows):
    scene_path = "C:\\Users\\rpadi\\Documents\\Dev\\PenumbraDev\\anim\\template\\toystory_new.pbrt"
    output_dir = "C:\\Users\\rpadi\\Documents\\Dev\\PenumbraDev\\anim\\gen\\toystory_final"
elif(mac):
    scene_path = "/Users/rafa/Documents/Dev/PenumbraDev/Penumbra/build-release/resources/scenes/materialcube.pbrt"
    output_dir = "/Users/rafa/Documents/Dev/PenumbraDev/Penumbra/resources/scenes/cube_anim"
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
            stripped_line = line.strip()
            if stripped_line.startswith('LookAt') and not stripped_line.startswith('#'):
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
