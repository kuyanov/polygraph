#!/usr/bin/env bash

BUILD_DIR=$1
PROJ_DIR=$(realpath $BUILD_DIR/..)
export CONF_DIR=$PROJ_DIR
export DATA_DIR=$PROJ_DIR

mkdir -p /var/log/polygraph

libsboxd start &
$BUILD_DIR/runner/prunner 0 all 2>>/var/log/polygraph/common.log &
sleep 1 && $BUILD_DIR/test/runner/test_prunner
STATUS=$?
pkill prunner
libsboxd stop
exit $STATUS
