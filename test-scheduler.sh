#!/usr/bin/env bash

polygraph start
sleep 1 && ./build/test/scheduler/test_scheduler
STATUS=$?
polygraph stop
exit $STATUS
