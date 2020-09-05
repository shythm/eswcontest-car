echo "Build the source"
make

echo "Copy the objects file to the remote device"
scp "build/ctrlboard" root@10.10.70.4:"bin/"
scp "build/recognize" root@10.10.70.4:"bin/"
scp "build/drive" root@10.10.70.4:"bin/"

echo "Initialize ths IPCs on the remote device."
ssh -t root@10.10.70.4 "ipcrm -Q 123;ipcrm -M 456"

echo "Execute program on the remote device"
ssh -t root@10.10.70.4 "bin/ctrlboard 123 & bin/recognize 456 123 & bin/drive 456 123;"

echo 'exit'