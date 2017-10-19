#####################################
# Controller Semantic make file
# Created on: 2016-09-09
# Author: Jugo
#####################################

include env.mk
include modules.mk

TARGET    := controller-semantic
CC        := g++
LD        := g++

SRC_DIR   := $(addprefix $(DIR_APP_ROOT),$(MODULES))
BUILD_DIR := $(addprefix $(DIR_APP_ROOT)build/,$(MODULES))
SRC       := $(filter-out %/main.cpp, $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.c*)))
OBJ       := $(patsubst $(DIR_APP_ROOT)%.cpp,$(DIR_APP_ROOT)build/%.o,$(SRC))
INCLUDES  := $(addprefix -I,$(SRC_DIR)) -I /usr/include/mysql -I /usr/include/curl -I /usr/local/include/xercesc
CXXFLAGS  := -Wall -Wno-unused-variable -fno-strict-aliasing $(LOG) -std=c++11 -g 
EXT_INCLUDES := $(addprefix -I,$(addprefix $(DIR_APP_ROOT),$(EXT_MODULES))) 

LDLIBS    :=-L$(DIR_APP_ROOT)libs \
			-L/lib64 \
			-L/usr/lib64 \
			-L/usr/lib64/x86_64-linux-gnu \
			-L/usr/lib/x86_64-linux-gnu \
			-L/usr/include \
			-L/usr/local/lib \
			-lpthread \
			-lm -lrt -ldl \
			-lmysqlclient \
			-lcurl \
			-lcryptopp \
			-lssl \
			-lcrypto \
			-lxerces-c \
			-lboost_thread \
			-lboost_system \
			-lboost_filesystem \
			-lboost_regex \
			-lboost_program_options
					
vpath %.cpp $(SRC_DIR) ../Global

define make-goal
$1/%.o: %.cpp
	$(CC) $(EXT_INCLUDES) $(INCLUDES) -c $$< -o $$@ $(CXXFLAGS) 
	@echo -ne '|==================== $1 Object Build Done. ====================|  '
	@echo `date`
	@echo "\n"
endef

.PHONY: all bin/$(TARGET) lib checkdirs clean cleanAll

.IGNORE: clean cleanAll

all: checkdirs bin/$(TARGET)
	@cp $(DIR_APP_ROOT)$(TARGET).conf $(DIR_APP_ROOT)bin
	@cp $(DIR_APP_ROOT)dictionary/*.* $(DIR_APP_ROOT)bin/dictionary
	@cp $(DIR_APP_ROOT)conf/*.* $(DIR_APP_ROOT)bin/conf
	@echo -ne '|==================== All Application Build Done. ====================|  '
	@echo `date`
	@echo "\n"

init: package-u checkdirs bin/$(TARGET)
	@cp $(DIR_APP_ROOT)$(TARGET).conf $(DIR_APP_ROOT)bin
	@echo -ne '|==================== All Application Build Done. ====================|  '
	@echo `date`
	@echo "\n"
	
bin/$(TARGET): $(OBJ)
	@echo -ne '|==================== Start Build $(TARGET) ====================|  '
	@echo `date`
	$(LD) $^ -o $@ $(CXXFLAGS) $(LDLIBS)
#	strip -s $@
	@echo "\n"

lib:
	@for libName in ${LIB_NAMES} ; do \
		$(MAKE) V=99 -C $$libName static; \
	done
	@echo -ne '|==================== All Library Build Done. ====================|  '
	@echo `date`
	@echo "\n"
		
checkdirs:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p bin
	@mkdir -p libs
	@mkdir -p bin/dictionary
	@mkdir -p bin/conf
	
clean:
	@for libName in ${MODULES} ; do \
		$(MAKE) V=99 -C $$libName clean > /dev/null; \
	done
	@for libName in ${LIB_NAMES} ; do \
		$(MAKE) V=99 -C $$libName clean > /dev/null; \
	done
	@for libName in ${EXT_MODULES} ; do \
		$(MAKE) V=99 -C $$libName clean > /dev/null; \
	done
	@rm -rf $(DIR_APP_ROOT)build
	@rm -rf $(DIR_APP_ROOT)bin
	@rm -rf $(DIR_APP_ROOT)log
	@rm -rf $(DIR_APP_ROOT)Global

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
	@echo -ne '|==================== Package for CentSO install completed. ====================|  '
	@echo `date`
	@echo "\n"
	
package-u:
	apt-get -y install sqlite3 libsqlite3-dev
	apt-get -y install libboost-all-dev libboost-thread-dev
	apt-get -y install build-essential
	apt-get -y install autoconf automake libtool
	apt-get -y install libmysqld-dev
	@echo -ne '|==================== Package for Ubuntu install completed. ====================|  '
	@echo `date`
	@echo "\n"
	
$(foreach bdir,$(BUILD_DIR),$(eval $(call make-goal,$(bdir))))

