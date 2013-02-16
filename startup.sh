make
cd tests/
make
cd ..
rm -f fyams.harddisk
util/tfstool create fyams.harddisk 2048 disk1
util/tfstool write fyams.harddisk tests/test_read test_read
yams buenos 'initprog=[disk1]test_read'
