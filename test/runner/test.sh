#!/bin/bash

export DATA_DIR=$PWD/data
export VAR_DIR=$PWD/var

./$BUILD_DIR/runner/runner --config config/runner_conf.json &
sleep 1 && ./$BUILD_DIR/test/runner/test_runner
STATUS=$?
pkill runner
exit $STATUS
