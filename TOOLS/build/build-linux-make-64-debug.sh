#!/bin/bash

SOURCE_PLATFORM=linux
TARGET_PLATFORM=linux
SIZE=64
GENERATOR='Unix Makefiles'
CONFIGURATION='Debug'

###############################
# DO NOT EDIT THE TEXT BELOW!
###############################

TARGET=$SOURCE_PLATFORM-$TARGET_PLATFORM-$SIZE-$CONFIGURATION
if [ ! -e BUILD ]; then
	mkdir BUILD
fi
cd BUILD
mkdir $TARGET
cd $TARGET

cmake -DTARGET_PLATFORM=$TARGET_PLATFORM -DTARGET_SIZE=$SIZE -DTARGET_DIR=$TARGET -G "$GENERATOR" ../..

DO_BUILD=1

if [ "$1" == "--no-build" ]; then
	DO_BUILD=0
fi

if [ $DO_BUILD -eq 1 ]; then
	cmake --build . --config $CONFIGURATION
fi

