#!/bin/bash
#set -x

ARCHS_TO_BUILD="arm64 x86_64"

# Find tools
CLANG=`xcrun --sdk macosx --find clang`
LIPO="xcrun -sdk macosx lipo"

ORIG_PATH=`pwd`
SRC_PATH=`pwd`/../libjpeg-turbo-2.0.6

################################################################################
ARCH=
CFLAGS=
ASMFLAGS=

function make_lib_for_arch() {
  # reset flags
  ARCH="$1"
  CFLAGS="-Wall -arch $ARCH -funwind-tables -fembed-bitcode"
  ASMFLAGS="-arch $ARCH"

  export CFLAGS="$CFLAGS $MIN_VER_FLAG"
  export ASMFLAGS="$ASMFLAGS"

  # Create arch folder
  ARCH_PATH="build-$ARCH"
  rm -rf $ARCH_PATH
  mkdir -p $ARCH_PATH
  cd $ARCH_PATH

  cat <<EOF> toolchain.cmake
    set(CMAKE_SYSTEM_NAME Darwin)
    set(CMAKE_SYSTEM_PROCESSOR aarch64)
    set(CMAKE_C_COMPILER ${CLANG})
EOF

  # Run CMake
  SDK_SYSROOT=`xcrun --sdk macosx --show-sdk-path`
  cmake -G"Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake -DCMAKE_OSX_SYSROOT=${SDK_SYSROOT} "${SRC_PATH}"
  if [ $? -ne 0 ] ; then
    echo "Failed to configure CMake for arch $ARCH"
    exit 1
  fi

  # Run make
  make
  if [ $? -ne 0 ] ; then
    echo "Failed to build for arch $ARCH"
    exit 1
  fi

  # Back to where we started...
  cd ..
}

LIPO_ARGS=
for arch in $ARCHS_TO_BUILD ; do
  make_lib_for_arch $arch
  LIPO_ARGS="$LIPO_ARGS -arch $arch build-$arch/libturbojpeg.a"
done

$LIPO $LIPO_ARGS -create -output libturbojpeg.a
file libturbojpeg.a
