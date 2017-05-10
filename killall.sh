#!/bin/sh

killall controller-dispatcher controller-signin controller-tracker controller-mongodb
sync;sync;sync
ps aux | grep controller
