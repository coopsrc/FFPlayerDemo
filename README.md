# FFPlayer

## ffmpeg build script for android

> 
> abi support: `armeabi-v7a` `arm64-v8a` `x86` `x86_64`
> ndk version android-ndk-r14b
>

### download ffmpeg
---
```
wget https://ffmpeg.org/releases/ffmpeg-4.0.tar.bz2
tar xvf ffmpeg-4.0.tar.bz2
```

### compile
---
``` bash
#!/bin/sh

PREFIX=android-build
HOST_PLATFORM=linux-x86_64

COMMON_OPTIONS="\
    --target-os=android \
    --disable-static \
    --enable-shared \
    --enable-small \
    --disable-programs \
    --disable-ffmpeg \
    --disable-ffplay \
    --disable-ffprobe \
    --disable-doc \
    --disable-symver \
    --disable-asm \
    --enable-decoder=vorbis \
    --enable-decoder=opus \
    --enable-decoder=flac 
    "

build_all(){
    for version in armeabi-v7a arm64-v8a x86 x86_64; do
        echo "======== > Start build $version"
        case ${version} in
        armeabi-v7a )
            ARCH="arm"
            CPU="armv7-a"
            CROSS_PREFIX="$NDK_HOME/toolchains/arm-linux-androideabi-4.9/prebuilt/$HOST_PLATFORM/bin/arm-linux-androideabi-"
            SYSROOT="$NDK_HOME/platforms/android-21/arch-arm/"
            EXTRA_CFLAGS="-march=armv7-a -mfpu=neon -mfloat-abi=softfp -mvectorize-with-neon-quad"
            EXTRA_LDFLAGS="-Wl,--fix-cortex-a8"
        ;;
        arm64-v8a )
            ARCH="aarch64"
            CPU="armv8-a"
            CROSS_PREFIX="$NDK_HOME/toolchains/aarch64-linux-android-4.9/prebuilt/$HOST_PLATFORM/bin/aarch64-linux-android-"
            SYSROOT="$NDK_HOME/platforms/android-21/arch-arm64/"
            EXTRA_CFLAGS=""
            EXTRA_LDFLAGS=""
        ;;
        x86 )
            ARCH="x86"
            CPU="i686"
            CROSS_PREFIX="$NDK_HOME/toolchains/x86-4.9/prebuilt/$HOST_PLATFORM/bin/i686-linux-android-"
            SYSROOT="$NDK_HOME/platforms/android-21/arch-x86/"
            EXTRA_CFLAGS=""
            EXTRA_LDFLAGS=""
        ;;
        x86_64 )
            ARCH="x86_64"
            CPU="x86_64"
            CROSS_PREFIX="$NDK_HOME/toolchains/x86_64-4.9/prebuilt/$HOST_PLATFORM/bin/x86_64-linux-android-"
            SYSROOT="$NDK_HOME/platforms/android-21/arch-x86_64/"
            EXTRA_CFLAGS=""
            EXTRA_LDFLAGS=""
        ;;
        esac

        echo "-------- > Start clean workspace"
        make clean

        echo "-------- > Start config makefile"
        configuration="\
            --prefix=${PREFIX} \
            --libdir=${PREFIX}/libs/${version}
            --incdir=${PREFIX}/includes/${version} \
            --pkgconfigdir=${PREFIX}/pkgconfig/${version} \
            --arch=${ARCH} \
            --cpu=${CPU} \
            --cross-prefix=${CROSS_PREFIX} \
            --sysroot=${SYSROOT} \
            --extra-ldexeflags=-pie \
            ${COMMON_OPTIONS}
            "

        echo "-------- > Start config makefile with ${configuration}"
        ./configure ${configuration}

        echo "-------- > Start make ${version} with -j8"
        make j8

        echo "-------- > Start install ${version}"
        make install
        echo "++++++++ > make and install ${version} complete."

    done
}

echo "-------- Start --------"
build_all
echo "-------- End --------"

```
