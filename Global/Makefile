#####################################
# Controller Module make file
# Created on: 2017-01-10
# Author: Jugo
# Project: Build Library
#####################################

include env.mk
include modules.mk
include libs.mk

TARGET    := controller-module
CC        := g++
LD        := g++

SRC_DIR   := $(addprefix $(DIR_APP_ROOT),$(MODULES))
BUILD_DIR := $(addprefix $(DIR_APP_ROOT)build/,$(MODULES))
SRC       := $(filter-out %/main.cpp, $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.c*)))
OBJ       := $(patsubst $(DIR_APP_ROOT)%.cpp,$(DIR_APP_ROOT)build/%.o,$(SRC))
INCLUDES  := $(addprefix -I,$(SRC_DIR)) $(LIB_INCLUDE) -I /usr/include/postgresql -I /usr/include/mysql -I /usr/include/curl
CXXFLAGS  := -Wall -Wno-unused-variable -g -fno-strict-aliasing $(LOG)
EXT_INCLUDES := $(addprefix -I,$(addprefix $(DIR_APP_ROOT),$(EXT_MODULES)))

LDLIBS    := $(LIB_LINK) \
			-L$(DIR_APP_ROOT)libs \
			-L/lib64 \
			-L/usr/lib64 \
			-L/usr/lib64/x86_64-linux-gnu \
			-L/usr/lib/x86_64-linux-gnu \
			-L/usr/include \
			-lpthread \
			-lm -lrt -ldl \
			-lpq \
			-lsqlite3 \
			-lmongoclient \
			-lboost_thread \
			-lboost_system \
			-lboost_filesystem \
			-lboost_regex \
			-lboost_program_options \
			-L/usr/local/lib \
			-lmysqlclient \
			-lcurl \
			-lcrypto++ \
			-ltinyxml2
					
vpath %.cpp $(SRC_DIR)

define make-goal
$1/%.o: %.cpp
	$(CC) $(INCLUDES) -c $$< -o $$@ $(CXXFLAGS) $(EXT_INCLUDES)
	@echo -ne '###### $1 Object Build Done. ######\n'
	@echo "\n"
endef

.PHONY: all bin/$(TARGET) lib checkdirs clean cleanAll

.IGNORE: clean cleanAll

all: checkdirs lib bin/$(TARGET)
	@echo -ne '\n###### All Application Build Done. ######\n'
	@echo "\n"
	
bin/$(TARGET): $(OBJ)
	@echo -ne '\n###### Start Build $(TARGET) ######\n'
	$(LD) $^ -o $@ $(CXXFLAGS) $(LDLIBS)
#	strip -s $@
	@echo "\n"

lib:
	@for libName in ${LIB_NAMES} ; do \
		$(MAKE) V=99 -C $$libName static; \
		mkdir -p $(DIR_APP_ROOT)libs/$$libName/heads; \
		cp $$libName/lib/*.a $(DIR_APP_ROOT)libs/$$libName; \
		cp $$libName/*.h $(DIR_APP_ROOT)libs/$$libName/heads; \
	done
	@echo -ne '\n###### All Library Build Done. ######\n'
	@echo "\n"
		
checkdirs:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p bin
	@mkdir -p libs
	
clean:
	@for libName in ${MODULES} ; do \
		$(MAKE) V=99 -C $$libName clean > /dev/null; \
	done
	@for libName in ${LIB_NAMES} ; do \
		$(MAKE) V=99 -C $$libName clean > /dev/null; \
	done
	@for libName in ${EXT_MODULES} ; do \
		$(MAKE) V=99 -C $$libName clean; \
	done
	@rm -rf $(DIR_APP_ROOT)build
	@rm -rf $(DIR_APP_ROOT)bin

cleanAll: clean
	@rm -rf $(DIR_APP_ROOT)libs
	
mk_debug:
	@echo "\n"
	@echo 'Program Name    : '$(TARGET)
	@echo 'Program Version : '$(VERSION)"\n"
	@echo 'Source Director : '$(SRC_DIR)"\n"
	@echo 'Source  : '$(SRC)"\n"
	@echo 'Build Director  : '$(BUILD_DIR)"\n"
	@echo 'Link Library    : '$(LDLIBS)"\n"
	@echo 'Extend Include :'$(EXT_INCLUDES)"\n"
	@echo 'Libs:'${LIB_NAMES}"\n"
	
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
	apt-get -y install libmongo-client-dev mongodb-clients
	apt-get -y install libbson-dev
	apt-get -y install libmongoc-dev
	apt-get -y install libpq-dev
	apt-get -y install libmysqld-dev
	apt-get -y install scons
	apt-get -y install libcurl4-openssl-dev
	apt-get -y install libcrypto++-dev
	apt-get -y install libtinyxml2-dev
	@echo -ne '\n###### Package for Ubuntu install completed. ######\n'
	@echo "\n"

debug:
	apt-get -y install libc6-dbg gdb valgrind valkyrie	
	@echo -ne '\n###### Package for Ubuntu Debug Tool install Completed. ######\n'
	@echo "\n"
	
mongodb:
	@cd $(DIR_APP_ROOT)extLibs/mongo-cxx-driver && scons install CCFLAGS="-Wno-unused-variable";
	@cp -Rf $(DIR_APP_ROOT)extLibs/mongo-cxx-driver/build/install/include/mongo $(DIR_APP_ROOT)extInclude
	@cp $(DIR_APP_ROOT)extLibs/mongo-cxx-driver/build/install/lib/libmongoclient.a $(DIR_APP_ROOT)libs/libmongoclient.a
	@echo -ne '\n###### MongoDB Build Done. ######\n'
	@echo "\n"
	
$(foreach bdir,$(BUILD_DIR),$(eval $(call make-goal,$(bdir))))
