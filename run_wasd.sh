set -e

echo "Build the source"
make ctrlboard
make clean-car
./CrossCompiler/gcc-linaro-arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc -o ./build/wasd ./src/wasd.c ./src/shared/ctrlboard-lib.c -I./src/shared/ -pthread

echo "Copy the object files to the remote device"
scp "build/ctrlboard" root@10.10.70.4:"bin/"
scp "build/wasd"      root@10.10.70.4:"bin/"

set +e

echo "Execute the programs on the remote device"
ssh -t root@10.10.70.4 "bin/ctrlboard & sleep .1;bin/wasd"

echo "Clean the programs on the remote device."
ssh -t root@10.10.70.4 "bin/clean-car"