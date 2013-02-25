rm -f fyams.harddisk
make
cd tests/
make
cd ..

util/tfstool create fyams.harddisk 2048 disk1
util/tfstool write fyams.harddisk tests/processTest processTest
util/tfstool write fyams.harddisk tests/hw hw
util/tfstool write fyams.harddisk tests/exec exec
yams buenos 'initprog=[disk1]exec' process_Debug

make clean
cd tests/
make clean
cd ..
