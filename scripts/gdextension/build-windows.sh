#!/bin/bash

OPTS="-j${NUM_CORES}"

echo "Running: scons $OPTS $SCONS_OPTS..."
scons $OPTS $SCONS_OPTS \
	|| exit 1
