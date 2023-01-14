#!/usr/bin/env bash

PROJ_DIR=$(realpath $BUILD_DIR/..)
export CONF_DIR=$PROJ_DIR/config
export DATA_DIR=$PROJ_DIR/data
export VAR_DIR=$PROJ_DIR/var

mkdir -p $PROJ_DIR/logs
$BUILD_DIR/scheduler/pscheduler 2>$PROJ_DIR/logs/scheduler.log &
sleep 1 && $BUILD_DIR/test/scheduler/test_pscheduler
STATUS=$?
pkill pscheduler
exit $STATUS
