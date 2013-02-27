rm -f fyams.harddisk
make
cd tests/
make
cd ..

util/tfstool create fyams.harddisk 2048 disk1
util/tfstool write fyams.harddisk tests/exec_1 exec_1
util/tfstool write fyams.harddisk tests/validprog validprog
util/tfstool write fyams.harddisk tests/prog1 prog1
util/tfstool write fyams.harddisk tests/prog2 prog2
util/tfstool write fyams.harddisk tests/join_1 join_1
yams buenos 'initprog=[disk1]exec_1' process_Debug

make clean
cd tests/
make clean
cd ..
