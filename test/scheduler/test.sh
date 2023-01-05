#!/bin/bash

export DATA_DIR=$PWD/data
export VAR_DIR=$PWD/var

./$BUILD_DIR/scheduler/scheduler --config config/scheduler_conf.json &
sleep 1 && ./$BUILD_DIR/test/scheduler/test_scheduler
STATUS=$?
pkill scheduler
exit $STATUS
