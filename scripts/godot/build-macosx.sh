#!/bin/bash

OPTS="-j${NUM_CORES} osxcross_sdk=darwin21.4"
STRIP="x86_64-apple-darwin21.4-strip -u -r"

case "$BUILD_TYPE" in
	*-x86-64)
		ARCH=x86_64
		;;
	*-arm64)
		ARCH=arm64
		;;
	*-universal)
		ARCH=universal
		;;
	*)
		echo "Unable to identify architecture from BUILD_TYPE: $BUILD_TYPE"
		exit 1
		;;
esac

if [ "$ARCH" = x86_64 -o "$ARCH" = universal ]; then
	echo "Running: scons $OPTS $SCONS_OPTS arch=x86_64..."
	scons $OPTS $SCONS_OPTS arch=x86_64 \
		|| exit 1
fi
if [ "$ARCH" = arm64 -o "$ARCH" = universal ]; then
	echo "Running: scons $OPTS $SCONS_OPTS arch=arm64..."
	scons $OPTS $SCONS_OPTS arch=arm64 \
		|| exit 1
fi
if [ "$ARCH" = universal ]; then
	lipo -create bin/godot.osx${FN_OPT}${FN_TOOLS}.x86_64 bin/godot.osx${FN_OPT}${FN_TOOLS}.arm64 -output bin/godot.osx${FN_OPT}${FN_TOOLS}.universal
	rm -f bin/godot.osx${FN_OPT}${FN_TOOLS}.x86_64
	rm -f bin/godot.osx${FN_OPT}${FN_TOOLS}.arm64
fi
$STRIP bin/godot.osx${FN_OPT}${FN_TOOLS}.$ARCH

pushd bin

if [ "$TARGET" = "editor" ]; then
	rm -rf Editor.app
	cp -r ../misc/dist/osx_tools.app Editor.app
	mkdir Editor.app/Contents/MacOS
	mv godot.osx${FN_OPT}.tools.$ARCH Editor.app/Contents/MacOS/Godot
	zip $BUILD_TYPE.zip $(find Editor.app)
	rm -rf Editor.app
else
	rm -rf osx_template.app
	cp -r ../misc/dist/osx_template.app ./
	mkdir osx_template.app/Contents/MacOS
	mv godot.osx${FN_OPT}.$ARCH osx_template.app/Contents/MacOS/godot_osx_release.64
	zip $BUILD_TYPE.zip $(find osx_template.app)
	rm -rf osx_template.app
fi

popd

