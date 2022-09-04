#!/bin/bash

./cmake-build-asan/scheduler/Scheduler 2>> stderr.log &
sleep 1 && ./cmake-build-asan/test/scheduler/TestScheduler
STATUS=$?
pkill Scheduler
cat stderr.log
exit $STATUS
