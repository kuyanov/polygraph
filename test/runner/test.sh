#!/bin/bash

export RESOURCES_PATH=$PWD/resources
export DATA_PATH=$PWD/data

./$BUILD_DIR/runner/runner --config runner/config.json &
sleep 1 && ./$BUILD_DIR/test/runner/test_runner
STATUS=$?
pkill runner
exit $STATUS
