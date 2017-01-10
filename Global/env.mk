#####################################
# global variable
# Created on: 2017-01-10
# Author: Jugo
#####################################

export VERSION=v0.0.0.2
export DIR_APP_ROOT=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))

#####################################
# Running Version module controle
# Build Debug version use: -DDEBUG
# Build Trace version use: -DTRACE
# Build Release version use: -DRELEASE
#####################################
export LOG=-DDEBUG 