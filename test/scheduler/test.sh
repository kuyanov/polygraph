#!/bin/bash

export DATA_DIR=$PWD/data
export VAR_DIR=$PWD/var

./$BUILD_DIR/scheduler/pscheduler --config scheduler/scheduler_conf.json &
sleep 1 && ./$BUILD_DIR/test/scheduler/test_pscheduler
STATUS=$?
pkill pscheduler
exit $STATUS
