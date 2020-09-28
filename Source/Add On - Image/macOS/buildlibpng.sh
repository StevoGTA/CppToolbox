# Builds a Libpng framework for macOS.
#===============================================================================
set -x

: ${LIB_VERSION:=1.6.37}

: ${SDKVERSION:=`xcodebuild -showsdks | grep macosx | egrep "[[:digit:]]+\.[[:digit:]]+" -o | tail -1`}
: ${XCODE_ROOT:=`xcode-select -print-path`}

: ${ORIGDIR:=`pwd`}
: ${TARBALLDIR:=`pwd`}
: ${SRCDIR:=`pwd`/src}
: ${BUILDDIR:=`pwd`/macOS/build}

rm -rf /tmp/build
ln -s "`pwd`/macOS/build" /tmp/build
: ${PREFIXDIR:=/tmp/build}

LIB_TARBALL="$TARBALLDIR/libpng-$LIB_VERSION.tar.xz"
LIB_SRC="$SRCDIR/libpng-${LIB_VERSION}"

#===============================================================================
ARM_DEV_CMD="xcrun --sdk macosx"

#===============================================================================
# Functions
#===============================================================================

abort()
{
    echo
    echo "Aborted: $@"
    exit 1
}

doneSection()
{
    echo
    echo "================================================================="
    echo "Done"
    echo
}

#===============================================================================

cleanEverythingReadyToStart()
{
    echo Cleaning everything before we start to build...

    rm -rf macOS-build
    rm -rf "$PREFIXDIR"

    doneSection
}

#===============================================================================

downloadLibpng()
{
    if [ ! -s "$LIB_TARBALL" ]; then
        echo "Downloading libpng ${LIB_VERSION}"
        curl -L -o "$LIB_TARBALL" http://sourceforge.net/projects/libpng/files/libpng16/${LIB_VERSION}/libpng-${LIB_VERSION}.tar.xz
    fi

    doneSection
}

#===============================================================================

unpackLibpng()
{
    [ -f "$LIB_TARBALL" ] || abort "Source tarball missing."

    echo Unpacking libpng into $SRCDIR...

    [ -d "$SRCDIR" ]    || mkdir -p "$SRCDIR"
    [ -d "$LIB_SRC" ] || ( cd "$SRCDIR"; tar xfj "$LIB_TARBALL" )
    [ -d "$LIB_SRC" ] && echo "    ...unpacked as $LIB_SRC"

    doneSection
}

#===============================================================================

buildLibpng()
{
    export CC=$XCODE_ROOT/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang
    export CC_BASENAME=clang

    export CXX=$XCODE_ROOT/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang++
    export CXX_BASENAME=clang++

    # avoid the `LDFLAGS` env to include the homebrew Cellar
    export LDFLAGS=""
    
    cd "$LIB_SRC"

    echo Building Libpng
    export CFLAGS="-O3 -arch arm64 -arch x86_64 -isysroot $XCODE_ROOT/Platforms/MacOSX.platform/Developer/SDKs/MacOSX${SDKVERSION}.sdk -Wno-error-implicit-function-declaration -fembed-bitcode"
    export CPPFLAGS="-arch arm64 -I$XCODE_ROOT/Platforms/MacOSX.platform/Developer/SDKs/MacOSX${SDKVERSION}.sdk/usr/include"
    make distclean
    ./configure --prefix="$PREFIXDIR/macOS-build" --disable-dependency-tracking --enable-static=yes --enable-shared=no
    make
    make install
    doneSection
}

#===============================================================================
# Execution starts here
#===============================================================================

mkdir -p "$BUILDDIR"

# cleanEverythingReadyToStart #may want to comment if repeatedly running during dev

echo "LIB_VERSION:       $LIB_VERSION"
echo "LIB_SRC:           $LIB_SRC"
echo "PREFIXDIR:         $PREFIXDIR"
echo "SDKVERSION:        $SDKVERSION"
echo "XCODE_ROOT:        $XCODE_ROOT"
echo

downloadLibpng
unpackLibpng
buildLibpng

echo "Copying includes"
cd "$ORIGDIR"
cp -r "$PREFIXDIR/macOS-build/include" .

echo "Lipo-ing library..."
rm -rf "lib"
mkdir "lib"
$ARM_DEV_CMD lipo -create $PREFIXDIR/*/lib/libpng16.a -o "lib/libpng16.a" || abort "Lipo $1 failed"

cd lib
ln -s libpng16.a libpng.a

echo "Completed successfully"
cd "$ORIGDIR"

#===============================================================================
