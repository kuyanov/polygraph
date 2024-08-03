#!/usr/bin/env bash

BUILD_DIR=$1

libsboxd start &
polygraph runner start
sleep 1 && $BUILD_DIR/test/runner/test_runner
STATUS=$?
polygraph runner stop
libsboxd stop
exit $STATUS
