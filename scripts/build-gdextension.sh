#!/bin/bash

die() {
	echo "$@" > /dev/stderr
	exit 1
}

if [ -z "$NUM_CORES" ]; then
	NUM_CORES=$(nproc --all)
fi

PRIVATE_IMAGE="no"
IMAGE=""
CMD=""
PLATFORM=""

case "$BUILD_TYPE" in
	linux-*)
		IMAGE="godot-linux"
		CMD="build-linux.sh"
		PLATFORM="linux"
		;;
	windows-*)
		IMAGE="godot-windows"
		CMD="build-windows.sh"
		PLATFORM="windows"
		;;
	macosx-*)
		IMAGE="godot-osx"
		PRIVATE_IMAGE="yes"
		CMD="build-macosx.sh"
		PLATFORM="osx"
		;;
	html5-*)
		IMAGE="godot-javascript"
		CMD="build-html5.sh"
		PLATFORM="web"
		;;
	*)
		die "Unable to identify platform from BUILD_TYPE: $BUILD_TYPE"
		;;
esac

ARCH=""
case "$BUILD_TYPE" in
	*-x86_32*)
		ARCH="x86_32"
		;;
	*-x86_64*)
		ARCH="x86_64"
		;;
esac

MONO=""
case "$BUILD_TYPE" in
	*-mono*)
		MONO="yes"
		;;
esac

SCONS_OPTS=${SCONS_OPTS:-arch=$ARCH}
SCONS_OPTS="$SCONS_OPTS platform=$PLATFORM"

TARGET=""
FN_TOOLS=""
FN_OPT=""
case "$BUILD_TYPE" in
	*-debug*)
		TARGET="export-template-debug"
		SCONS_OPTS="$SCONS_OPTS target=template_debug"
		;;

	*-release*)
		TARGET="export-template-release"
		SCONS_OPTS="$SCONS_OPTS target=template_release"
		;;

	*)
		die "Unable to identify target from BUILD_TYPE: $BUILD_TYPE"
		;;
esac

if [ -n "$GODOT_BUILD_REGISTRY" ]; then
	# In the registry, godot-linux becomes godot/linux.
	IMAGE=$(echo "$IMAGE" | sed -e 's,-,/,')
	IMAGE="$GODOT_BUILD_REGISTRY/$IMAGE"
	if [ "$PRIVATE_IMAGE" = yes ]; then
		IMAGE=$(echo "$IMAGE" | sed -e 's,/godot/,/godot-private/,')
	fi
fi
if [ -n "$GODOT_BUILD_TAG" ]; then
	IMAGE="$IMAGE:$GODOT_BUILD_TAG"
fi

PODMAN_OPTS=${PODMAN_OPTS:-}

podman run --rm --systemd=false -v "$(pwd):/build" -v "$(pwd)/scripts/gdextension:/scripts" -w /build -e NUM_CORES="$NUM_CORES" -e PLATFORM="$PLATFORM" -e ARCH="$ARCH" -e MONO="$MONO" -e TARGET="$TARGET" -e FN_TOOLS="$FN_TOOLS" -e FN_OPT="$FN_OPT" -e "SCONS_OPTS=$SCONS_OPTS" -e BUILD_TYPE=$BUILD_TYPE $PODMAN_OPTS "$IMAGE" /scripts/$CMD $BUILD_TYPE
exit $?

