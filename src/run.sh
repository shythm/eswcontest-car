echo "Build source"

cd shared
make clean
make
cd ../

cd recognize
make clean
make
cd ../

echo "Copy object file to remote device"
scp "shared/ctrlboard" root@10.10.70.4:"/home/root/bin/ctrlboard"
scp "recognize/recognize" root@10.10.70.4:"/home/root/bin/recognize"

echo "Execute object file on remote device"
ssh -t root@10.10.70.4 "ipcrm -Q 123;ipcrm -M 456"
ssh -t root@10.10.70.4 "/home/root/bin/ctrlboard 123 & /home/root/bin/recognize 456 123;"