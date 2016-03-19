#!/bin/bash

#
# This is a make helper script for running with travis-ci.  It gets around the
#    4mb log output limit and does periodic pings to avoid the 10 min timeout.
#
# This script only executes "make" in the folder that it is in.  To use it, copy
#    this script to the build folder in the main Travis script and execute it.
#
# Based on: https://stackoverflow.com/questions/26082444/how-to-work-around-travis-cis-4mb-output-limit
#

# Abort on Error:
set -e

export PING_SLEEP=300s
export WORKDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
export BUILD_OUTPUT=$WORKDIR/build.out

touch $BUILD_OUTPUT

dump_output() {
   echo Tailing the last 500 lines of output:
   tail -500 $BUILD_OUTPUT  
}
error_handler() {
  echo ERROR: An error was encountered with the build.
  dump_output
  exit 1
}
# If an error occurs, run our error handler to output a tail of the build
trap 'error_handler' ERR

# Set up a repeating loop to send some output to Travis.

bash -c "while true; do echo \$(date) - building ...; sleep $PING_SLEEP; done" &
PING_LOOP_PID=$!

# This script should be copied and run from the target build folder!!
make -j 4 >> $BUILD_OUTPUT 2>&1

# The build finished without returning an error so dump a tail of the output
dump_output

# nicely terminate the ping output loop
kill $PING_LOOP_PID

