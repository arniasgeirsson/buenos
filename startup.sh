make
cd tests/
make
cd ..
rm -f fyams.harddisk
util/tfstool create fyams.harddisk 2048 disk1
util/tfstool write fyams.harddisk tests/testIO testIO

yams buenos 'initprog=[disk1]testIO'
