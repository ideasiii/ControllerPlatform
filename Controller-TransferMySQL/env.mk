#####################################
# global variable
# Created on: 2017-01-13
# Author: Jugo
#####################################

export VERSION=v0.0.0.1
export DIR_APP_ROOT=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))

#####################################
# Running Version module controle
# Build Debug version use: -DDEBUG
# Build Trace version use: -DTRACE
# Build Release version use: -DRELEASE
#####################################
export LOG=-DDEBUG

######################################
#
#	sync all user: 		-DSYNCALL_USER
#	sync all tracker:	-DSYNCALL_TRACKER
######################################
export RUN=-DSYNCALL_TRACKER 