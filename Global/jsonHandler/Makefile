#####################################
# cJSON Test make file
# Created on: 2016-07-26
# Author: Jugo
#####################################
TARGET    := jsontest
CC        := g++
LD        := g++

DIR_APP_ROOT := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
SRC_DIR   := $(DIR_APP_ROOT)
BUILD_DIR := $(DIR_APP_ROOT)
SRC       := $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)*.c*))
OBJ       := $(patsubst %.cpp, %.o,$(SRC))
INCLUDES  := $(addprefix -I,$(SRC_DIR)) -I./ -I../global_inc

CXXFLAGS  := -Wall -Wno-unused-variable -g -fno-strict-aliasing -DDEBUG
		
LDLIBS    :=-L$(DIR_APP_ROOT)libs \
			-L/lib64 \
			-L/usr/lib64 \
			-L/usr/lib64/x86_64-linux-gnu \
			-lpthread \
			-lm -lrt -ldl
	
vpath %.cpp $(SRC_DIR)

define make-goal
$1%.o: %.cpp
	$(CC) $(EXT_INCLUDES) $(INCLUDES) -c $$< -o $$@ $(CXXFLAGS) 
	@echo -ne '###### $1 Object Build Done. ######\n'
	@echo "\n"
endef

.PHONY: all bin/$(TARGET) checkdirs clean

.IGNORE: clean

all: checkdirs bin/$(TARGET)
	@echo -ne '\n###### $(TARGET) Build Done. ######\n'
	@echo "\n"

bin/$(TARGET): $(OBJ)
	@echo -ne '\n###### Start Build $(OBJ) ######\n'
	$(LD) $^ -o $@ $(CXXFLAGS) $(LDLIBS)
	strip -s $@
	@echo "\n"
		
checkdirs:
	@mkdir -p bin
	
clean:
	@rm -rf *.o
	@rm -rf $(DIR_APP_ROOT)bin
		
mk_debug:
	@echo "\n"
	@echo 'Program Name    : '$(TARGET)
	@echo 'Source Director : '$(SRC_DIR)"\n"
	@echo 'Build Director  : '$(BUILD_DIR)"\n"
	@echo 'Source  : '$(SRC)"\n"
	@echo 'Build   : '$(OBJ)"\n"
	@echo 'INCLUDE   : '$(INCLUDES)"\n"
		
$(foreach bdir,$(BUILD_DIR),$(eval $(call make-goal,$(bdir))))
