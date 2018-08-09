#######################
# socket handler make file
#######################

CC        := g++
LD        := g++

TARGET = socketHandler

DIR_APP_ROOT = $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
OBJDIR = $(DIR_APP_ROOT)build/$(TARGET)
OUTDIR = $(DIR_APP_ROOT)bin
LIBOUTDIR = $(DIR_APP_ROOT)lib
CXXFLAGS = -Wall -g $(LOG)
CINC   = -I./ -I../global_inc -I../threadHandler -I../messageHandler -I../objectHandler -I../dataHandler -I../logHandler
LDLIBS  = -L/usr/lib/x86_64-linux-gnu -lpthread -lm -lrt -ldl

_OBJS   = CSocket.o CSocketServer.o CSocketClient.o
OBJECTS = $(patsubst %, $(OBJDIR)/%, $(_OBJS))

all: $(TARGET)
	@echo -ne 'all application build Done.\n'
	
$(TARGET) : build_common object message thread data log $(OBJECTS)
	$(CC) $(CXXFLAGS)  $(OBJECTS)  $(LDLIBS)  -o $(OUTDIR)/$(TARGET) \
	./build/threadHandler/CThreadHandler.o \
	./build/messageHandler/CMessageHandler.o \
	./build/objectHandler/CObject.o \
	./build/dataHandler/CDataHandler.o \
	./build/logHandler/LogHandler.o

static: build_common lib$(TARGET).a
	@echo -ne 'static lib build Done.\n'

lib$(TARGET).a: $(OBJECTS)
	 ar rcs -o $(LIBOUTDIR)/lib$(TARGET).a $(OBJECTS)
	 
$(OBJDIR)/%.o : %.cpp
	$(CC) $(CXXFLAGS) -c $(CINC) -o $@ $<
	
thread: ../threadHandler/CThreadHandler.cpp
	$(CC) $(CXXFLAGS) -c $(CINC) -o ./build/threadHandler/CThreadHandler.o $<
	
message: ../messageHandler/CMessageHandler.cpp
	$(CC) $(CXXFLAGS) -c $(CINC) -o ./build/messageHandler/CMessageHandler.o $<
	
object: ../objectHandler/CObject.cpp
	$(CC) $(CXXFLAGS) -c $(CINC) -o ./build/objectHandler/CObject.o $<
	
data: ../dataHandler/CDataHandler.cpp
	$(CC) $(CXXFLAGS) -c $(CINC) -o ./build/dataHandler/CDataHandler.o $<
	
log: ../logHandler/LogHandler.cpp
	$(CC) $(CXXFLAGS) -c $(CINC) -o ./build/logHandler/CDataHandler.o $<
	
build_common:
	@[ -d $(OBJDIR) ] || mkdir -p $(OBJDIR) || false
	@[ -d $(OUTDIR) ] || mkdir -p $(OUTDIR) || false
	@[ -d $(LIBOUTDIR) ] || mkdir -p $(LIBOUTDIR) || false
	@[ -d "build/threadHandler" ] || mkdir -p "build/threadHandler" || false
	@[ -d "build/messageHandler" ] || mkdir -p "build/messageHandler" || false
	@[ -d "build/objectHandler" ] || mkdir -p "build/objectHandler" || false
	@[ -d "build/logHandler" ] || mkdir -p "build/logHandler" || false
	
clean:
	@rm -rf $(OBJDIR) $(OUTDIR)/$(TARGET) $(LIBOUTDIR) $(DIR_APP_ROOT)build $(DIR_APP_ROOT)bin

.PHONY: all static build_common server client thread message log