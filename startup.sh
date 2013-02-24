rm -f fyams.harddisk
make
cd tests/
make
cd ..

util/tfstool create fyams.harddisk 2048 disk1
util/tfstool write fyams.harddisk tests/processTest processTest
yams buenos 'initprog=[disk1]processTest'

make clean
cd tests/
make clean
cd ..
