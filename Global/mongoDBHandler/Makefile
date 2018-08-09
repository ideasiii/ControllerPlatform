#######################
# cmp handler make file
#######################


CC        := g++
LD        := g++

TARGET = mongoDBHandler

DIR_APP_ROOT = $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
OBJDIR = $(DIR_APP_ROOT)build/$(TARGET)
OUTDIR = $(DIR_APP_ROOT)bin
LIBOUTDIR = $(DIR_APP_ROOT)lib
CXXFLAGS = -Wall -g $(LOG)
CINC   = -I./ -I../global_inc -I../logHandler -I../extInclude -I../threadHandler
LDLIBS  = -L/usr/lib/x86_64-linux-gnu \
          -lpthread -lm -lrt -ldl \
          -L../libs \
          -L$(DIR_APP_ROOT)libs \
			-L/lib64 \
			-L/usr/lib64 \
			-L/usr/lib64/x86_64-linux-gnu \
			-I/usr/include/mongo \
			-lpthread \
			-lm -lrt -ldl \
			-lmongoclient \
			-lboost_thread \
			-lboost_system \
			-lboost_filesystem \
			-lboost_regex \
			-lboost_program_options

_OBJS   = CMongoDBHandler.o #main.o
OBJECTS = $(patsubst %, $(OBJDIR)/%, $(_OBJS))

all: $(TARGET)
	@echo -ne 'all application build Done.\n'
	
$(TARGET) : dir log thread extern $(OBJECTS)
	$(CC) $(CXXFLAGS)  $(OBJECTS)  $(LDLIBS)  -o $(OUTDIR)/$(TARGET) \
	./build/threadHandler/CThreadHandler.o \
	./build/logHandler/LogHandler.o \
	./build/global_inc/extern.o

static: dir lib$(TARGET).a
	@echo -ne 'static lib build Done.\n'

lib$(TARGET).a: $(OBJECTS)
	 ar rcs -o $(LIBOUTDIR)/lib$(TARGET).a $(OBJECTS)
	 
$(OBJDIR)/%.o : %.cpp
	$(CC) $(CXXFLAGS) -c $(CINC) -o $@ $<

thread: ../threadHandler/CThreadHandler.cpp
	$(CC) $(CXXFLAGS) -c $(CINC) -o ./build/threadHandler/CThreadHandler.o $<
		
log: ../logHandler/LogHandler.cpp
	$(CC) $(CXXFLAGS) -c $(CINC) -o ./build/logHandler/LogHandler.o $<
	
extern: ../global_inc/extern.cpp
	$(CC) $(CXXFLAGS) -c $(CINC) -o ./build/global_inc/extern.o $<
	
dir:
	@[ -d $(OBJDIR) ] || mkdir -p $(OBJDIR) || false
	@[ -d $(OUTDIR) ] || mkdir -p $(OUTDIR) || false
	@[ -d $(LIBOUTDIR) ] || mkdir -p $(LIBOUTDIR) || false
	@[ -d "build/threadHandler" ] || mkdir -p "build/threadHandler" || false
	@[ -d "build/logHandler" ] || mkdir -p "build/logHandler" || false
	[ -d "build/global_inc" ] || mkdir -p "build/global_inc" || false
	
clean:
	@rm -rf $(OBJDIR) $(OUTDIR)/$(TARGET) $(LIBOUTDIR) $(DIR_APP_ROOT)build $(DIR_APP_ROOT)bin

.PHONY: all static dir log thread extern