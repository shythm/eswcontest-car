echo "Build the source"
# Each time cmake is rebuilt so that you can use a script containing the source code automatically.
cmake CMakeLists.txt
make

echo "Copy the objects file to the remote device"
scp "build/ctrlboard" root@10.10.70.4:"bin/"
scp "build/recognize" root@10.10.70.4:"bin/"
scp "build/clean-car" root@10.10.70.4:"bin/"
scp "build/control"   root@10.10.70.4:"bin/"

echo "Execute the programs on the remote device"
ssh -t root@10.10.70.4 "bin/ctrlboard & bin/recognize & bin/control"

echo "Clean the programs on the remote device."
ssh -t root@10.10.70.4 "bin/clean-car"
