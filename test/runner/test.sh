#!/bin/bash

PROJ_DIR=$(realpath $BUILD_DIR/..)
export DATA_DIR=$PROJ_DIR/data
export VAR_DIR=$PROJ_DIR/var

./$BUILD_DIR/runner/prunner --config $PROJ_DIR/runner/runner_conf.json 2>stderr.log &
sleep 1 && ./$BUILD_DIR/test/runner/test_prunner
STATUS=$?
pkill prunner
cat stderr.log && rm stderr.log
exit $STATUS
