#!/bin/sh

echo 'kill controller-sematic process'
killall controller-semantic
sync;sync;sync
echo 'build source'
make clean all
echo 'build finish'
cd bin
echo 'start run semantic process'
ipcrm -q 0
./controller-semantic &
echo '^____^Y Bye Bye'