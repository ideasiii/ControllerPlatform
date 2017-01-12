#######################
# cmp handler make file
#######################

CC = g++
TARGET = cmpHandler

DIR_APP_ROOT = $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
OBJDIR = $(DIR_APP_ROOT)build/$(TARGET)
OUTDIR = $(DIR_APP_ROOT)bin
LIBOUTDIR = $(DIR_APP_ROOT)lib
CXXFLAGS = -Wall -g $(LOG)
CINC   = -I./ -I../global_inc -I../dataHandler -I../logHandler -I../socketHandler
LDLIBS  = -L/usr/lib/x86_64-linux-gnu -lpthread -lm -lrt -ldl

_OBJS   = CCmpHandler.o
OBJECTS = $(patsubst %, $(OBJDIR)/%, $(_OBJS))

all: $(TARGET)
	@echo -ne 'all application build Done.\n'
	
$(TARGET) : dir data log socket $(OBJECTS)
	$(CC) $(CXXFLAGS)  $(OBJECTS)  $(LDLIBS)  -o $(OUTDIR)/$(TARGET) \
	./build/dataHandler/CDataHandler.o \
	./build/logHandler/LogHandler.o \
	./build/socketHandler/CSocket.o

static: dir lib$(TARGET).a
	@echo -ne 'static lib build Done.\n'

lib$(TARGET).a: $(OBJECTS)
	 ar rcs -o $(LIBOUTDIR)/lib$(TARGET).a $(OBJECTS)
	 
$(OBJDIR)/%.o : %.cpp
	$(CC) $(CXXFLAGS) -c $(CINC) -o $@ $<

data: ../dataHandler/CDataHandler.cpp
	$(CC) $(CXXFLAGS) -c $(CINC) -o ./build/dataHandler/CDataHandler.o $<
	
log: ../logHandler/LogHandler.cpp
	$(CC) $(CXXFLAGS) -c $(CINC) -o ./build/logHandler/CDataHandler.o $<
	
socket: ../socketHandler/CSocket.cpp
	$(CC) $(CXXFLAGS) -c $(CINC) -o ./build/socketHandler/CSocket.o $<
	
dir:
	@[ -d $(OBJDIR) ] || mkdir -p $(OBJDIR) || false
	@[ -d $(OUTDIR) ] || mkdir -p $(OUTDIR) || false
	@[ -d $(LIBOUTDIR) ] || mkdir -p $(LIBOUTDIR) || false
	@[ -d "build/logHandler" ] || mkdir -p "build/logHandler" || false
	@[ -d "build/socketHandler" ] || mkdir -p "build/socketHandler" || false
	
clean:
	@rm -rf $(OBJDIR) $(OUTDIR)/$(TARGET) $(LIBOUTDIR) $(DIR_APP_ROOT)build $(DIR_APP_ROOT)bin

.PHONY: all static dir log data