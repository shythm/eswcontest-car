set -e
CARGCC=./CrossCompiler/gcc-linaro-arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc

echo "Build the source"
$CARGCC -o ./build/pure-psd-drive ./src/pure-psd-drive.c -I./src/shared -lm
make clean-car

echo "Copy the objects file to the remote device"
scp "build/pure-psd-drive" root@10.10.70.4:"bin/"
scp "build/clean-car"      root@10.10.70.4:"bin/"

set +e

echo "Execute the programs on the remote device"
ssh -t root@10.10.70.4 "bin/pure-psd-drive"

echo "Clean the programs on the remote device."
ssh -t root@10.10.70.4 "bin/clean-car"
