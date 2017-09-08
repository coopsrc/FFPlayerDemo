# FFPlayer

## ffmpeg build script for android

> 
> abi support: `armeabi` `armeabi-v7a` `arm64-v8a` `x86` `x86_64`
>

### download ffmpeg
---
```
wget http://ffmpeg.org/releases/ffmpeg-3.3.2.tar.bz2
tar xvf ffmpeg-3.3.2.tar.bz2
```

### compile
---
``` bash
PREFIX=android-build-debug
HOST_PLATFORM=windows-x86_64

COMMON_OPTIONS="\
    --prefix=${PREFIX}/ \
    --target-os=android \
    --enable-shared \
    --enable-avresample \
    --enable-small \
    --enable-cross-compile \
    --disable-static \
    --disable-doc \
    --disable-programs \
    --disable-ffmpeg \
    --disable-ffplay \
    --disable-ffprobe \
    --disable-ffserver \
    --disable-symver \
    --disable-yasm \
    "

function build_android {
    ./configure \
    --libdir=${PREFIX}/libs/armeabi \
    --incdir=${PREFIX}/includes/armeabi \
    --pkgconfigdir=${PREFIX}/pkgconfig/armeabi \
    --arch=arm \
    --cpu=armv6 \
    --cross-prefix="${NDK_PATH}/toolchains/arm-linux-androideabi-4.9/prebuilt/${HOST_PLATFORM}/bin/arm-linux-androideabi-" \
    --sysroot="${NDK_PATH}/platforms/android-21/arch-arm/" \
    --extra-ldexeflags=-pie \
    ${COMMON_OPTIONS}
    make clean
    make -j4 && make install

    ./configure \
    --libdir=${PREFIX}/libs/armeabi-v7a \
    --incdir=${PREFIX}/includes/armeabi-v7a \
    --pkgconfigdir=${PREFIX}/pkgconfig/armeabi-v7a \
    --arch=arm \
    --cpu=armv7-a \
    --cross-prefix="${NDK_PATH}/toolchains/arm-linux-androideabi-4.9/prebuilt/${HOST_PLATFORM}/bin/arm-linux-androideabi-" \
    --sysroot="${NDK_PATH}/platforms/android-21/arch-arm/" \
    --extra-cflags="-march=armv7-a -mfloat-abi=softfp -mfpu=neon" \
    --extra-ldflags="-Wl,--fix-cortex-a8" \
    --extra-ldexeflags=-pie \
    ${COMMON_OPTIONS}
    make clean
    make -j4 && make install

    ./configure \
    --libdir=${PREFIX}/libs/arm64-v8a \
    --incdir=${PREFIX}/includes/arm64-v8a \
    --pkgconfigdir=${PREFIX}/pkgconfig/arm64-v8a \
    --arch=aarch64 \
    --cpu=armv8-a \
    --cross-prefix="${NDK_PATH}/toolchains/aarch64-linux-android-4.9/prebuilt/${HOST_PLATFORM}/bin/aarch64-linux-android-" \
    --sysroot="${NDK_PATH}/platforms/android-21/arch-arm64/" \
    --extra-ldexeflags=-pie \
    ${COMMON_OPTIONS} 
    make clean
    make -j4 && make install

    ./configure \
    --libdir=${PREFIX}/libs/x86 \
    --incdir=${PREFIX}/includes/x86 \
    --pkgconfigdir=${PREFIX}/pkgconfig/x86 \
    --arch=x86 \
    --cpu=i686 \
    --cross-prefix="${NDK_PATH}/toolchains/x86-4.9/prebuilt/${HOST_PLATFORM}/bin/i686-linux-android-" \
    --sysroot="${NDK_PATH}/platforms/android-21/arch-x86/" \
    --extra-ldexeflags=-pie \
    --disable-asm \
    ${COMMON_OPTIONS} 
    make clean
    make -j4 && make install

    ./configure \
    --libdir=${PREFIX}/libs/x86_64 \
    --incdir=${PREFIX}/includes/x86_64 \
    --pkgconfigdir=${PREFIX}/pkgconfig/x86_64 \
    --arch=x86_64 \
    --cpu=x86_64 \
    --cross-prefix="${NDK_PATH}/toolchains/x86_64-4.9/prebuilt/${HOST_PLATFORM}/bin/x86_64-linux-android-" \
    --sysroot="${NDK_PATH}/platforms/android-21/arch-x86_64/" \
    --extra-ldexeflags=-pie \
    --disable-asm \
    ${COMMON_OPTIONS}
    make clean
    make -j4 && make install
}

build_android
```