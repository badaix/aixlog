#!/bin/bash

if [[ -z $ANDROID_NDK ]] ; then
	echo "ANDROID_NDK" must point to the Android NDK root directory
	exit 1
fi


export CROSS_COMPILE=$ANDROID_NDK/bin/arm-linux-androideabi-
export CC=$ANDROID_NDK/bin/arm-linux-androideabi-gcc
export CXX=$ANDROID_NDK/bin/arm-linux-androideabi-g++
export CXXFLAGS="-U_ARM_ASSEM_ -I$ANDROID_NDK/sysroot/usr/include -DANDROID -llog -pie"

mkdir -p build
cd build
cmake ..
make
