#!/usr/bin/env bash

NDK_HOME=/opt/android/android-ndk-r15c
PREFIX=android-build
HOST_PLATFORM=linux-x86_64
PLATFORM=android-21
CONFIG_LOG_PATH=${PREFIX}/log

COMMON_OPTIONS="\
--target-os=android \
--disable-static \
--enable-shared \
--enable-small \
--enable-cross-compile \
--disable-programs \
--disable-ffmpeg \
--disable-ffplay \
--disable-ffprobe \
--disable-doc \
--disable-symver \
--enable-decoder=h264 \
--enable-decoder=mpeg4 \
--enable-decoder=mjpeg \
--enable-decoder=png \
--enable-decoder=vorbis \
--enable-decoder=opus \
--enable-decoder=flac "

build(){
    APP_ABI=$1
    ARCH=$2
    CPU=$3
    MARCH=$4
    echo "======== > Start build $APP_ABI $ARCH $CPU $MARCH"
    case ${APP_ABI} in
    armeabi )
        CROSS_PREFIX="$NDK_HOME/toolchains/arm-linux-androideabi-4.9/prebuilt/$HOST_PLATFORM/bin/arm-linux-androideabi-"
        SYSROOT="$NDK_HOME/platforms/$PLATFORM/arch-arm"
        EXTRA_CFLAGS="-march=$MARCH"
        EXTRA_LDFLAGS=""
        EXTRA_OPTIONS="--disable-x86asm"
    ;;
    armeabi-v7a )
        CROSS_PREFIX="$NDK_HOME/toolchains/arm-linux-androideabi-4.9/prebuilt/$HOST_PLATFORM/bin/arm-linux-androideabi-"
        SYSROOT="$NDK_HOME/platforms/$PLATFORM/arch-arm"
        EXTRA_CFLAGS="-march=$MARCH -mfloat-abi=softfp"
        EXTRA_LDFLAGS="-Wl,--fix-cortex-a8"
        EXTRA_OPTIONS="--enable-neon --disable-x86asm"
    ;;
    arm64-v8a )
        CROSS_PREFIX="$NDK_HOME/toolchains/aarch64-linux-android-4.9/prebuilt/$HOST_PLATFORM/bin/aarch64-linux-android-"
        SYSROOT="$NDK_HOME/platforms/$PLATFORM/arch-arm64"
        EXTRA_CFLAGS="-march=$MARCH"
        EXTRA_LDFLAGS=""
        EXTRA_OPTIONS="--enable-neon --disable-x86asm"
    ;;
    x86 )
        CROSS_PREFIX="$NDK_HOME/toolchains/x86-4.9/prebuilt/$HOST_PLATFORM/bin/i686-linux-android-"
        SYSROOT="$NDK_HOME/platforms/$PLATFORM/arch-x86"
        EXTRA_CFLAGS="-march=$MARCH"
        EXTRA_LDFLAGS=""
        EXTRA_OPTIONS="--disable-asm"
    ;;
    x86_64 )
        CROSS_PREFIX="$NDK_HOME/toolchains/x86_64-4.9/prebuilt/$HOST_PLATFORM/bin/x86_64-linux-android-"
        SYSROOT="$NDK_HOME/platforms/$PLATFORM/arch-x86_64"
        EXTRA_CFLAGS="-march=$MARCH"
        EXTRA_LDFLAGS=""
        EXTRA_OPTIONS="--disable-asm"
    ;;
    esac

    echo "-------- > Start clean workspace"
    make clean

    echo "-------- > Start build configuration"
    configuration="\
--logfile=$CONFIG_LOG_PATH/config_$APP_ABI.log \
--prefix=$PREFIX \
--libdir=$PREFIX/libs/$APP_ABI \
--incdir=$PREFIX/includes/$APP_ABI \
--pkgconfigdir=$PREFIX/pkgconfig/$APP_ABI \
--arch=$ARCH \
--cpu=$CPU \
--cross-prefix=$CROSS_PREFIX \
--sysroot=$SYSROOT \
--extra-cflags='$EXTRA_CFLAGS' \
--extra-ldflags='$EXTRA_LDFLAGS' \
--extra-ldexeflags=-pie \
$EXTRA_OPTIONS \
$COMMON_OPTIONS
"

    echo "-------- > Start config makefile with $configuration"
    ./configure \
--logfile=${CONFIG_LOG_PATH}/config_${APP_ABI}.log \
--prefix=${PREFIX} \
--libdir=${PREFIX}/libs/${APP_ABI} \
--incdir=${PREFIX}/includes/${APP_ABI} \
--pkgconfigdir=${PREFIX}/pkgconfig/${APP_ABI} \
--arch=${ARCH} \
--cpu=${CPU} \
--cross-prefix=${CROSS_PREFIX} \
--sysroot=${SYSROOT} \
--extra-cflags="$EXTRA_CFLAGS" \
--extra-ldflags="$EXTRA_LDFLAGS" \
--extra-ldexeflags=-pie \
${EXTRA_OPTIONS} \
${COMMON_OPTIONS}

    echo "-------- > Start make $APP_ABI with -j8"
    make j8

    echo "-------- > Start install $APP_ABI"
    make install
    echo "++++++++ > make and install $APP_ABI complete."

}

build_all(){
    mkdir -p ${CONFIG_LOG_PATH}

#    build $app_abi $arch $cpu $march
    build "armeabi" "arm" "armv5te" "armv5te"
    build "armeabi-v7a" "arm" "armv7-a" "armv7-a"
    build "arm64-v8a" "aarch64" "armv8-a" "armv8-a"
    build "x86" "x86" "i686" "i686"
    build "x86_64" "x86_64" "x86_64" "x86-64"
}

echo "-------- Start --------"
build_all
echo "-------- End --------"