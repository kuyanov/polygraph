#!/usr/bin/env bash

PROJ_DIR=$(realpath $BUILD_DIR/..)
export CONF_DIR=$PROJ_DIR/config
export DATA_DIR=$PROJ_DIR/data
export VAR_DIR=$PROJ_DIR/var

mkdir -p $VAR_DIR/log
$BUILD_DIR/scheduler/pscheduler 2>$VAR_DIR/log/pscheduler.log &
sleep 1 && $BUILD_DIR/test/scheduler/test_pscheduler
STATUS=$?
pkill pscheduler
exit $STATUS
