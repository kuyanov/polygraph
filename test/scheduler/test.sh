#!/usr/bin/env bash

PROJ_DIR=$(realpath $BUILD_DIR/..)
export CONF_DIR=$PROJ_DIR/config
export DATA_DIR=$PROJ_DIR/data

mkdir -p /var/log/polygraph
$BUILD_DIR/scheduler/pscheduler 2>>/var/log/polygraph/common.log &
sleep 1 && $BUILD_DIR/test/scheduler/test_pscheduler
STATUS=$?
pkill pscheduler
exit $STATUS
