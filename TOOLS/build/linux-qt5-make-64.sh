#!/bin/bash

QTVER=5
SOURCE_PLATFORM=linux
TARGET_PLATFORM=linux
SIZE=64
GENERATOR='Unix Makefiles'

###############################
# DO NOT EDIT THE TEXT BELOW!
###############################

QT=qt$QTVER
TARGET=$SOURCE_PLATFORM-$TARGET_PLATFORM-$QT-$SIZE
if [ ! -e BUILD ]; then
	mkdir BUILD
fi
cd BUILD
mkdir $TARGET
cd $TARGET

cmake -DTARGET_PLATFORM=$TARGET_PLATFORM -DTARGET_SIZE=$SIZE -DQT=$QTVER -DTARGET_DIR=$TARGET -G "$GENERATOR" ../..

DO_BUILD=1

if [ "$1" == "--no-build" ]; then
	DO_BUILD=0
fi

if [ $DO_BUILD -eq 1 ]; then
	cmake --build . --config Debug
	cmake --build . --config Release
fi

