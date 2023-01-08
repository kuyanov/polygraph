#!/bin/bash

PROJ_DIR=$(realpath $BUILD_DIR/..)
export CONF_FILE=$PROJ_DIR/config.json
export DATA_DIR=$PROJ_DIR/data
export VAR_DIR=$PROJ_DIR/var

./$BUILD_DIR/scheduler/pscheduler 2>stderr.log &
sleep 1 && ./$BUILD_DIR/test/scheduler/test_pscheduler
STATUS=$?
pkill pscheduler
cat stderr.log && rm stderr.log
exit $STATUS
