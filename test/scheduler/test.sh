#!/bin/bash

export RESOURCES_PATH=$PWD/resources
export DATA_PATH=$PWD/data

./$BUILD_DIR/scheduler/scheduler --config scheduler/config.json &
sleep 1 && ./$BUILD_DIR/test/scheduler/test_scheduler
STATUS=$?
pkill scheduler
exit $STATUS
