#####################################
# build library
# Created on: 2017-01-10
# Author: Jugo
#####################################

# set your lib folder name here.
#LIB_NAMES := httpServerHandler \
#			 socketHandler
			 
LIB_NAMES := \
socketHandler \
cmpHandler \
configHandler \
dataHandler \
mongoDBHandler \
logHandler
				   
LIB_INCLUDE := $(addprefix -I,$(LIB_NAMES))

LIB_LINK := $(foreach bdir,$(LIB_NAMES),-L$(DIR_APP_ROOT)$(bdir)/lib -l$(bdir))

