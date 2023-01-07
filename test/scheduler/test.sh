#!/bin/bash

PROJ_DIR=$(realpath $BUILD_DIR/..)
export DATA_DIR=$PROJ_DIR/data
export VAR_DIR=$PROJ_DIR/var

mkdir -p $PROJ_DIR/logs
$BUILD_DIR/scheduler/pscheduler --config $PROJ_DIR/scheduler/scheduler_conf.json 2>$PROJ_DIR/logs/scheduler.log &
sleep 1 && $BUILD_DIR/test/scheduler/test_pscheduler
STATUS=$?
pkill pscheduler
exit $STATUS
