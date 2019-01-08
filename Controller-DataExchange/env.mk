#####################################
# global variable
# Created on: 2016-05-09
# Author: Jugo
#####################################

export VERSION=v0.0.0.1
export DIR_APP_ROOT=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))

#####################################
# Version module controle
# Build Debug version use: -DDEBUG -DTRACE -DTRACE_BODY
# Build Release version use: -DRELEASE
#####################################
export LOG=-DDEBUG