[Mongo DB]
# Run mongodb
#mongod -auth
mongod

# Set Authentication
#use admin
#db.addUser("admin","admin")

# Run mongodb command shell
mongo

# Run mongodb access shell
use access
#db.auth("admin","admin")
db.mobile.find()				// query record
db.mobile.remove({})			// remove all record


[sqlite]
sqlite3 xxxx.db

[Socket]
# Check socket status
netstat -npt
ss -t -a

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
while true; do date; ps aux -T | grep controller-semantic ; sleep 3; done;

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

# vi replace word
%s/[A-Z]\[edit\]/\1/g
%s/\[.*\]/\1/g
%s/\(.*\)/\1/g
删除行尾空格：:%s/\s+$//g
删除行首多余空格：%s/^\s*// 或者 %s/^ *//
删除沒有內容的空行：%s/^$// 或者 g/^$/d
删除包含有空格组成的空行：%s/^\s*$// 或者 g/^\s*$/d
删除以空格或TAB开头到结尾的空行：%s/^[ |\t]*$// 或者 g/^[ |\t]*$/d

# Download Youtube (順安)
youtube-dl --extract-audio --audio-format mp3 'https://www.youtube.com/watch?v=DdskNY5ayfc'

valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all bin/controller-semantic
valkyrie bin/controller-semantic