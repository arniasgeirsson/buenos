rm -f fyams.harddisk
make
cd tests/
make
cd ..

util/tfstool create fyams.harddisk 2048 disk1
util/tfstool write fyams.harddisk tests/readwrite readwrite
yams buenos 'initprog=[disk1]readwrite'

make clean
cd tests/
make clean
cd ..
