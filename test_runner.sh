#!/usr/bin/env bash

RUN_DIR=/var/run/polygraph
mkdir -p $RUN_DIR
touch $RUN_DIR/scheduler.pid
libsboxd start &
polygraph runner start
sleep 1 && ./build/test/runner/test_runner
STATUS=$?
polygraph runner stop
libsboxd stop
rm $RUN_DIR/scheduler.pid
exit $STATUS
