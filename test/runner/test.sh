#!/bin/bash

export DATA_DIR=$PWD/data
export VAR_DIR=$PWD/var

./$BUILD_DIR/runner/polyrunner --config runner/runner_conf.json &
sleep 1 && ./$BUILD_DIR/test/runner/test_polyrunner
STATUS=$?
pkill polyrunner
exit $STATUS
