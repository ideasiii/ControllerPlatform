[Process]
# Check ELF File
readelf -a xxxx

# test child process reload
kill -11 [child process pid]

# Check Linking Library
ldd xxx

# 用 strace 執行程式, 觀察開啟的檔案: strace -f -e open PROGRAM 2>&1 | grep "\.so"

# 在程式執行中觀察 /proc/PID/maps, 這個檔案記錄 process 用到的各區段記憶體為何, 可從對應到的檔案看出有載入的 shared library

#  monitor memory usage
while true; do date; ps aux | grep controller-signin ; sleep 1; done;

# List Socket Connect
lsof | grep controlCenter

# List File Description
/proc/PID/fd

# Use apt-get to install mongodb
apt-get install mongodb-dev libmongo-client-dev

[IPC]
#list IPC
ipcs -q
#remove IPC
ipcrm -q [msq_id]

# 檢查Thread數目
while true; do date; ps -T -a | grep controller-disp ; sleep 1; done;

# Thread 壓測
while true; do date; ./simulator 127.0.0.1 2306 & ; sleep 1; done;