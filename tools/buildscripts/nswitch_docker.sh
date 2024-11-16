#!/bin/bash

git config --global --add safe.directory '*'

echo "Setting up environment..."

# we can't actually download dkp-toolchain-vars even from here, so
export PORTLIBS_ROOT=${DEVKITPRO}/portlibs
export PATH=${DEVKITPRO}/tools/bin:${DEVKITPRO}/devkitA64/bin:$PATH
export TOOL_PREFIX=aarch64-none-elf-
export CC=${TOOL_PREFIX}gcc
export CXX=${TOOL_PREFIX}g++
export AR=${TOOL_PREFIX}gcc-ar
export RANLIB=${TOOL_PREFIX}gcc-ranlib
export PORTLIBS_PREFIX=${DEVKITPRO}/portlibs/switch
export PATH=$PORTLIBS_PREFIX/bin:$PATH
export ARCH="-march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIC -ftls-model=local-exec"
export CFLAGS="${ARCH} -O2 -ffunction-sections -fdata-sections"
export CXXFLAGS="${CFLAGS}"
export CPPFLAGS="-D__SWITCH__ -I${PORTLIBS_PREFIX}/include -isystem ${DEVKITPRO}/libnx/include"
export LDFLAGS="${ARCH} -L${PORTLIBS_PREFIX}/lib -L${DEVKITPRO}/libnx/lib"
export LIBS="-lnx"

echo "Configuring..."

aarch64-none-elf-cmake -G"Unix Makefiles" -B$1 $2 $3 $4 $5 . || exit 1

echo "Building..."

pushd $1 || exit 1
make -j4 || exit 1
popd || exit 1
