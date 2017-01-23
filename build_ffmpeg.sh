cd ffmpeg 
./configure --enable-runtime-cpudetect --disable-programs --disable-doc --disable-avdevice --disable-avformat --enable-dxva2 --disable-everything --enable-decoder=hevc --disable-debug
make
cd ..
