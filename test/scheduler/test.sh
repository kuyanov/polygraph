#!/bin/bash

export DATA_DIR=$PWD/data
export VAR_DIR=$PWD/var

./$BUILD_DIR/scheduler/polyscheduler --config scheduler/scheduler_conf.json &
sleep 1 && ./$BUILD_DIR/test/scheduler/test_polyscheduler
STATUS=$?
pkill polyscheduler
exit $STATUS
