make
cd tests/
make
cd ..

util/tfstool create fyams.harddisk 2048 disk1
util/tfstool write fyams.harddisk tests/testIO testIO

yams buenos 'initprog=[disk1]testIO'

make clean
cd tests/
make clean
cd ..
rm -f fyams.harddisk
