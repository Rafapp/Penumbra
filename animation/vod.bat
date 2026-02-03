@echo off
ffmpeg -framerate 24 -i %%d.png -c:v libx264 -pix_fmt yuv420p output.mp4
pause