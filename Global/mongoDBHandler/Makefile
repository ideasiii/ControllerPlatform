#######################
# cmp handler make file
#######################

CC = g++
TARGET = mongoDBHandler

DIR_APP_ROOT = $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
OBJDIR = $(DIR_APP_ROOT)build/$(TARGET)
OUTDIR = $(DIR_APP_ROOT)bin
LIBOUTDIR = $(DIR_APP_ROOT)lib
CXXFLAGS = -Wall -g $(LOG)
CINC   = -I./ -I../global_inc -I../logHandler -I../extInclude
LDLIBS  = -L/usr/lib/x86_64-linux-gnu -lpthread -lm -lrt -ldl -L../libs

_OBJS   = CMongoDBHandler.o
OBJECTS = $(patsubst %, $(OBJDIR)/%, $(_OBJS))

all: $(TARGET)
	@echo -ne 'all application build Done.\n'
	
$(TARGET) : dir data log socket $(OBJECTS)
	$(CC) $(CXXFLAGS)  $(OBJECTS)  $(LDLIBS)  -o $(OUTDIR)/$(TARGET) \
	./build/logHandler/LogHandler.o

static: dir lib$(TARGET).a
	@echo -ne 'static lib build Done.\n'

lib$(TARGET).a: $(OBJECTS)
	 ar rcs -o $(LIBOUTDIR)/lib$(TARGET).a $(OBJECTS)
	 
$(OBJDIR)/%.o : %.cpp
	$(CC) $(CXXFLAGS) -c $(CINC) -o $@ $<
	
log: ../logHandler/LogHandler.cpp
	$(CC) $(CXXFLAGS) -c $(CINC) -o ./build/logHandler/CDataHandler.o $<
	
dir:
	@[ -d $(OBJDIR) ] || mkdir -p $(OBJDIR) || false
	@[ -d $(OUTDIR) ] || mkdir -p $(OUTDIR) || false
	@[ -d $(LIBOUTDIR) ] || mkdir -p $(LIBOUTDIR) || false
	@[ -d "build/logHandler" ] || mkdir -p "build/logHandler" || false
	
clean:
	@rm -rf $(OBJDIR) $(OUTDIR)/$(TARGET) $(LIBOUTDIR) $(DIR_APP_ROOT)build $(DIR_APP_ROOT)bin

.PHONY: all static dir log data