#!/usr/bin/env bash

PROJ_DIR=$(realpath $BUILD_DIR/..)
export CONF_DIR=$PROJ_DIR/config
export DATA_DIR=$PROJ_DIR/data
export VAR_DIR=$PROJ_DIR/var

mkdir -p $PROJ_DIR/logs
$BUILD_DIR/runner/prunner 2>$PROJ_DIR/logs/runner.log &
sleep 1 && $BUILD_DIR/test/runner/test_prunner
STATUS=$?
pkill prunner
exit $STATUS
