#!/bin/bash

OPTS="-j${NUM_CORES}"

if [ "$BITS" = "32" ]; then
	export PATH="${GODOT_SDK_LINUX_X86}/bin:${BASE_PATH}"
else
	export PATH="${GODOT_SDK_LINUX_X86_64}/bin:${BASE_PATH}"
fi

echo "Running: scons $OPTS $SCONS_OPTS..."
scons $OPTS $SCONS_OPTS \
	|| exit 1
