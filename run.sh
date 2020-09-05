echo "Build the source"
make

echo "Copy the objects file to the remote device"
scp "build/ctrlboard" root@10.10.70.4:"/home/root/bin/ctrlboard"
scp "build/recognize" root@10.10.70.4:"/home/root/bin/recognize"
scp "build/clean-car" root@10.10.70.4:"/home/root/bin/clean-car"

echo "Execute the programs on the remote device"
ssh -t root@10.10.70.4 "/home/root/bin/ctrlboard & /home/root/bin/recognize"

echo "Clean the programs on the remote device."
ssh -t root@10.10.70.4 "/home/root/bin/clean-car"