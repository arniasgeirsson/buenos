rm -f fyams.harddisk
make
cd tests/
make
cd ..

util/tfstool create fyams.harddisk 2048 disk1
util/tfstool write fyams.harddisk tests/test_write test_write
util/tfstool write fyams.harddisk tests/test_read test_read
util/tfstool write fyams.harddisk tests/test_io test_io


yams buenos 'initprog=[disk1]test_write'
yams buenos 'initprog=[disk1]test_read'
yams buenos 'initprog=[disk1]test_io'
make clean
cd tests/
make clean
cd ..
