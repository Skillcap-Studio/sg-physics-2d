#!/bin/bash

die() {
	echo "$@" >/dev/stderr
	exit 1
}

WINDOWS_EXE="$1"
APPLICATION_NAME="$2"
APPLICATION_URL="$3"

if [ -z "$WINDOWS_EXE" -o -z "$APPLICATION_NAME" -o -z "$APPLICATION_URL" ]; then
	die "Usage: windows-sign.sh <path-to-exe> <application-name> <application-url>"
fi

WINDOWS_EXE_SIGNED="$WINDOWS_EXE.signed"

if [ -z "$WINDOWS_SIGNING_CERT" -o -z "$WINDOWS_SIGNING_KEY" ]; then
	die "Missing required variable"
fi

echo "$WINDOWS_SIGNING_CERT" > code_signing.crt
echo "$WINDOWS_SIGNING_KEY" > code_signing.key

osslsigncode sign -certs code_signing.crt -key code_signing.key -n "$APPLICATION_NAME" -i "$APPLICATION_URL" -in "$WINDOWS_EXE" -out "$WINDOWS_EXE_SIGNED" \
	|| die "Unable to sign Windows EXE file"

mv "$WINDOWS_EXE_SIGNED" "$WINDOWS_EXE"

