#!/bin/bash
#set -x

ARCHS_TO_BUILD="armv7 armv7s arm64 x86_64"

# LIPO is used to combine static libraries
LIPO="xcrun -sdk iphoneos lipo"

# Need to include local path for gas-preprocessor.pl
PATH="$PATH:$(PWD)"
CPUS=$(sysctl -n hw.logicalcpu_max)

IOS_SDK_MIN_VERSION=9.0

ORIG_PATH=`pwd`
SRC_PATH=`pwd`/../libjpeg-turbo-2.0.4

################################################################################
IOS_GCC=`xcrun --sdk iphoneos --find clang`
ARCH=
CFLAGS=
ASMFLAGS=

function make_lib_for_arch() {
  # reset flags
  ARCH="$1"
  CFLAGS=
  ASMFLAGS=
  
  if [ $ARCH == "armv7" ] || [ $ARCH == "armv7s" ]
  then
    # 32-bit build (armv7 and armv7s)
    CFLAGS="-mfloat-abi=softfp -arch $ARCH -fembed-bitcode"
    ASMFLAGS="-no-integrated-as"
  else
    # 64-bit build (arm64 and x86_64)
    CFLAGS="-Wall -arch $ARCH -miphoneos-version-min=7.0 -funwind-tables -fembed-bitcode"
    ASMFLAGS=""
  fi

  if [ $ARCH == "arm64" ] || [ $ARCH == "armv7" ] || [ $ARCH == "armv7s" ]
  then
    # Device build
    SDK=iphoneos
    MIN_VER_FLAG="-miphoneos-version-min=$IOS_SDK_MIN_VERSION"
  else
    # Simulator build
    SDK=iphonesimulator
    MIN_VER_FLAG="-mios-simulator-version-min=$IOS_SDK_MIN_VERSION"
  fi

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
    set(CMAKE_C_COMPILER ${IOS_GCC})
EOF

  # Run CMake
  IOS_SYSROOT=`xcrun --sdk $SDK --show-sdk-path`
  cmake -G"Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake -DCMAKE_OSX_SYSROOT=${IOS_SYSROOT} "${SRC_PATH}"
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
