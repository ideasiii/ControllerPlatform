#####################################
# make file
# Created on: 2016-07-26
# Author: Jugo
#####################################


.PHONY: all clean

all:
	@echo -ne '\n###### Just Global Module not to Build ######\n'
	@echo "\n"
	
clean:
	@echo -ne '\n###### Just Global Module not to Clean ######\n'
	@echo "\n"
	
package-s:
	yum -y install sqlite-devel.x86_64
	yum -y install boost boost-devel
	yum -y install gcc-c++
	@echo -ne '\n###### Package for CentSO install completed. ######\n'
	@echo "\n"
	
package-u:
	apt-get -y install sqlite3 libsqlite3-dev
	apt-get -y install libboost-all-dev libboost-thread-dev
	apt-get -y install build-essential
	apt-get -y install autoconf automake libtool
	apt-get -y install libpq-dev
	@echo -ne '\n###### Package for Ubuntu install completed. ######\n'
	@echo "\n"

