#!/bin/bash

GODOT_SOURCE_DIR=${GODOT_BUILD_DIR:-src}
GODOT_BUILD_DIR=${GODOT_BUILD_DIR:-build/godot}
TEMP_DIR=$(mktemp -d)

die() {
	echo "$@" > /dev/stderr
	exit 1
}

if [ -z "$BUILD_TYPE" ]; then
	die "The BUILD_TYPE environment variable must be set"
fi

if [ ! -d "$GODOT_SOURCE_DIR" ]; then
	die "No such GODOT_SOURCE_DIR directory at $GODOT_SOURCE_DIR"
fi

if [ -z "$GODOT_DOWNLOAD_URL" ]; then
	die "Source directory is missing required GODOT_DOWNLOAD_URL file"
fi

if [ ! -d "$GODOT_BUILD_DIR" ]; then
	mkdir -p "$GODOT_BUILD_DIR" \
		|| die "Unable to create GODOT_BUILD_DIR: $GODOT_BUILD_DIR"

	fn=$(basename "$GODOT_DOWNLOAD_URL")
	(cd "$TEMP_DIR" && wget "$GODOT_DOWNLOAD_URL")
	(cd "$GODOT_BUILD_DIR" && tar -xf "$TEMP_DIR/$fn" --strip-components=1) \
		|| die "Unable to download Godot source from GODOT_DOWNLOAD_URL: $GODOT_DOWNLOAD_URL"
else
	echo " !! WARNING: Reusing existing build directory !! "
fi

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
		PLATFORM="x11"
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
		PLATFORM="javascript"
		;;
	*)
		die "Unable to identify platform from BUILD_TYPE: $BUILD_TYPE"
		;;
esac

BITS=""
case "$BUILD_TYPE" in
	*-32*)
		BITS="32"
		;;
	*-64*)
		BITS="64"
		;;
esac

MONO=""
case "$BUILD_TYPE" in
	*-mono*)
		MONO="yes"
		;;
esac

SCONS_OPTS=${SCONS_OPTS:-production=yes}
SCONS_OPTS="$SCONS_OPTS platform=$PLATFORM custom_modules=/src"

TARGET=""
FN_TOOLS=""
FN_OPT=".opt"
case "$BUILD_TYPE" in
	*-editor*)
		TARGET="editor"
		SCONS_OPTS="$SCONS_OPTS tools=yes target=release_debug"
		FN_TOOLS=".tools"
		;;

	*-export-template-debug*)
		TARGET="export-template-debug"
		SCONS_OPTS="$SCONS_OPTS tools=no  target=release_debug"
		FN_OPT=".opt.debug"
		;;

	*-export-template-release*)
		TARGET="export-template-release"
		SCONS_OPTS="$SCONS_OPTS tools=no  target=release"
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

(cd $GODOT_BUILD_DIR && patch -p1 < $CI_PROJECT_DIR/src/sg_physics_2d/godot-3/convert_float_contextual_menu.patch)

podman run --rm --systemd=false -v "$(realpath $GODOT_BUILD_DIR):/build" -v "$(realpath $GODOT_SOURCE_DIR):/src" -v "$(pwd)/scripts/godot:/scripts" -w /build -e NUM_CORES="$NUM_CORES" -e PLATFORM="$PLATFORM" -e BITS="$BITS" -e MONO="$MONO" -e TARGET="$TARGET" -e FN_TOOLS="$FN_TOOLS" -e FN_OPT="$FN_OPT" -e "SCONS_OPTS=$SCONS_OPTS" -e BUILD_TYPE=$BUILD_TYPE $PODMAN_OPTS "$IMAGE" /scripts/$CMD $BUILD_TYPE
exit $?

