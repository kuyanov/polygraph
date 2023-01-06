#!/bin/bash

export DATA_DIR=$PWD/data
export VAR_DIR=$PWD/var

./$BUILD_DIR/runner/prunner --config runner/runner_conf.json &
sleep 1 && ./$BUILD_DIR/test/runner/test_prunner
STATUS=$?
pkill prunner
exit $STATUS
