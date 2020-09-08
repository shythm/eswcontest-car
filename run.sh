#!/bin/bash
# Set exit on error
set -e

echo "Build the source"
# Each time cmake is rebuilt so that you can use a script containing the source code automatically.
cmake CMakeLists.txt
make

echo "Copy the objects file to the remote device"
scp "build/ctrlboard" root@10.10.70.4:"bin/"
scp "build/recognize" root@10.10.70.4:"bin/"
scp "build/clean-car" root@10.10.70.4:"bin/"
scp "build/process"   root@10.10.70.4:"bin/"

# Because -e flag was set, initialization before start is required.
echo "Initialize ipcs on the remote device."
ssh -t root@10.10.70.4 "bin/clean-car"

echo "Execute the programs on the remote device"
ssh -t root@10.10.70.4 "bin/ctrlboard & bin/recognize & bin/process"

echo "Clean the programs on the remote device."
ssh -t root@10.10.70.4 "bin/clean-car"
