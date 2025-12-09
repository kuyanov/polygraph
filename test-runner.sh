#!/usr/bin/env bash

libsboxd start &
polygraph runner start
sleep 1 && ./build/test/runner/test_runner
STATUS=$?
polygraph runner stop
libsboxd stop
exit $STATUS
