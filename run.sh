#!/bin/bash
# Set exit on error
set -e

APP=${1:-run}
echo $APP

echo "Build the source"
# Each time cmake is rebuilt so that you can use a script containing the source code automatically.
cmake CMakeLists.txt
make

echo "Copy the objects file to the remote device"
scp "build/recognize" root@10.10.70.4:"bin/"
scp "build/clean-car" root@10.10.70.4:"bin/"
scp "build/process"   root@10.10.70.4:"bin/"
scp "build/wasd"      root@10.10.70.4:"bin/"

set +e

echo "Execute the programs on the remote device"
if [ "$APP" == "run" ]; then
    ssh -t root@10.10.70.4 "bin/recognize & bin/process"
fi
if [ "$APP" == "wasd" ]; then
    ssh -t root@10.10.70.4 "bin/wasd"
fi

echo "Clean the programs on the remote device."
ssh -t root@10.10.70.4 "bin/clean-car"
