#!/bin/bash

PROJ_DIR=$(realpath $BUILD_DIR/..)
export DATA_DIR=$PROJ_DIR/data
export VAR_DIR=$PROJ_DIR/var

mkdir -p $PROJ_DIR/logs
$BUILD_DIR/runner/prunner --config $PROJ_DIR/runner/runner_conf.json 2>$PROJ_DIR/logs/runner.log &
sleep 1 && $BUILD_DIR/test/runner/test_prunner
STATUS=$?
pkill prunner
exit $STATUS
