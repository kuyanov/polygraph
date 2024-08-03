#!/usr/bin/env bash

BUILD_DIR=$1

polygraph start
sleep 1 && $BUILD_DIR/test/scheduler/test_scheduler
STATUS=$?
polygraph stop
exit $STATUS
