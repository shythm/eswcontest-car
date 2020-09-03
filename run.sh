echo "Build the source"
make

echo "Copy the objects file to the remote device"
scp "build/ctrlboard" root@10.10.70.4:"/home/root/bin/ctrlboard"
scp "build/recognize" root@10.10.70.4:"/home/root/bin/recognize"

echo "Initialize ths IPCs on the remote device."
ssh -t root@10.10.70.4 "ipcrm -Q 123;ipcrm -M 456"

echo "Execute program on the remote device"
ssh -t root@10.10.70.4 "/home/root/bin/ctrlboard 123 & /home/root/bin/recognize 456 123;"