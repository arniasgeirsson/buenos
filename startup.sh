rm -f fyams.harddisk
make
cd tests/
make
cd ..

util/tfstool create fyams.harddisk 2048 disk1

#util/tfstool write fyams.harddisk tests/exec_1 exec_1
#util/tfstool write fyams.harddisk tests/join_1 join_1
#util/tfstool write fyams.harddisk tests/exit_1 exit_1
#util/tfstool write fyams.harddisk tests/validprog validprog
#util/tfstool write fyams.harddisk tests/prog1 prog1
#util/tfstool write fyams.harddisk tests/prog2 prog2
#util/tfstool write fyams.harddisk tests/prog3 prog3
#util/tfstool write fyams.harddisk tests/process_test test
#yams buenos 'initprog=[disk1]test' #process_Debug
util/tfstool write fyams.harddisk tests/G4_test test
yams buenos 'initprog=[disk1]test' debug_G4

make clean
cd tests/
make clean
cd ..
