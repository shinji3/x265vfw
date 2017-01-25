# The builds depend on the used compiler (which is on the PATH first)
# x265vfw configure needs an extra x64 config for proper working

./build_ffmpeg.sh
./build_x265vfw.sh
