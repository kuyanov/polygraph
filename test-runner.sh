#!/usr/bin/env bash

PID_FILE=/var/run/polygraph/scheduler.pid
touch $PID_FILE
libsboxd start &
polygraph runner start
sleep 1 && ./build/test/runner/test_runner
STATUS=$?
polygraph runner stop
libsboxd stop
rm $PID_FILE
exit $STATUS
