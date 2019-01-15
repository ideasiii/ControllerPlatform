#####################################
# Controller make file
# Created on: 2019-01-07
# Author: Jugo
# This controller is for data exchange
#####################################

include env.mk
include modules.mk

TARGET    := controller-dataexchange
CC        := g++
LD        := g++

SRC_DIR   := $(addprefix $(DIR_APP_ROOT),$(MODULES))
BUILD_DIR := $(addprefix $(DIR_APP_ROOT)build/,$(MODULES))
SRC       := $(filter-out %/main.cpp, $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.c*)))
OBJ       := $(patsubst $(DIR_APP_ROOT)%.cpp,$(DIR_APP_ROOT)build/%.o,$(SRC))
INCLUDES  := $(addprefix -I,$(SRC_DIR)) -I /usr/local/include/mongocxx/v_noabi/ -I /usr/local/include/bsoncxx/v_noabi/
CXXFLAGS  := -Wall -Wno-unused-variable -fno-strict-aliasing $(LOG) -g -std=c++11
EXT_INCLUDES := $(addprefix -I,$(addprefix $(DIR_APP_ROOT),$(EXT_MODULES))) 

LDLIBS    :=-L$(DIR_APP_ROOT)libs \
			-L/lib64 \
			-L/usr/lib64 \
			-L/usr/lib64/x86_64-linux-gnu \
			-L/usr/local/lib \
			-lpthread \
			-lm -lrt -ldl \
			-lmongocxx \
			-lbsoncxx
			
vpath %.cpp $(SRC_DIR)

define make-goal
$1/%.o: %.cpp
	$(CC) $(EXT_INCLUDES) $(INCLUDES) -c $$< -o $$@ $(CXXFLAGS) 
	@echo -ne '###### $1 Object Build Done. ######\n'
	@echo "\n"
endef

.PHONY: all bin/$(TARGET) lib checkdirs clean cleanAll

.IGNORE: clean

all: checkdirs lib bin/$(TARGET)
	@cp $(DIR_APP_ROOT)$(TARGET).conf $(DIR_APP_ROOT)bin
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
	done
	@echo -ne '\n###### All Library Build Done. ######\n'
	@echo "\n"
		
checkdirs:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p bin
	@mkdir -p libs
	
clean:
	@for libName in ${LIB_NAMES} ; do \
		$(MAKE) V=99 -C $$libName clean; \
	done
	@for libName in ${EXT_MODULES} ; do \
		$(MAKE) V=99 -C $$libName clean; \
	done
	@rm -rf $(DIR_APP_ROOT)build
	@rm -rf $(DIR_APP_ROOT)bin
	@rm -rf $(DIR_APP_ROOT)log
	@rm -rf $(DIR_APP_ROOT)Global

	
mk_debug:
	@echo "\n"
	@echo 'Program Name    : '$(TARGET)
	@echo 'Program Version : '$(VERSION)"\n"
	@echo 'Source Director : '$(SRC_DIR)"\n"
	@echo 'Build Director  : '$(BUILD_DIR)"\n"
	@echo 'Link Library    : '$(LDLIBS)"\n"
	@echo 'Extend Include :'$(EXT_INCLUDES)"\n"
		
package:
	apt-get -y install build-essential
	apt-get -y install autoconf automake libtool
	@echo -ne '\n###### Package for Ubuntu install completed. ######\n'
	@echo "\n"


$(foreach bdir,$(BUILD_DIR),$(eval $(call make-goal,$(bdir))))

