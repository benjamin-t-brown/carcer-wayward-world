#!/bin/sh

export THIS_DIR=$(pwd)

# export SDL_VIDEODRIVER=mmiyoo
# export SDL_AUDIODRIVER=mmiyoo
# export EGL_VIDEODRIVER=mmiyoo
export LD_LIBRARY_PATH="$THIS_DIR:/customer/lib:/lib:/lib:/config/lib:/mnt/SDCARD/miyoo/lib:/mnt/SDCARD/.tmp_update/lib:/mnt/SDCARD/.tmp_update/lib/parasyte"
cd $THIS_DIR
PID=$(pidof MainUI)
echo $PID
kill -STOP $PID
./SCOUNDREL
kill -CONT $PID