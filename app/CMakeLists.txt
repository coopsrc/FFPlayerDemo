cmake_minimum_required(VERSION 3.4.1)

find_library( log-lib
              log )

add_library( native-lib
             SHARED
             src/main/cpp/native-lib.cpp )

set(distribution_DIR ${CMAKE_SOURCE_DIR}/../../../../src/main/jniLibs)


add_library( avcodec
             SHARED
             IMPORTED )
set_target_properties( avcodec
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../src/main/jniLibs/${ANDROID_ABI}/libavcodec.so )

add_library( avfilter
             SHARED
             IMPORTED )
set_target_properties( avfilter
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../src/main/jniLibs/${ANDROID_ABI}/libavfilter.so )

add_library( swscale
             SHARED
             IMPORTED )
set_target_properties( swscale
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../src/main/jniLibs/${ANDROID_ABI}/libswscale.so )

add_library( avformat
             SHARED
             IMPORTED )
set_target_properties( avformat
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../src/main/jniLibs/${ANDROID_ABI}/libavformat.so )

add_library( avutil
             SHARED
             IMPORTED )
set_target_properties( avutil
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../src/main/jniLibs/${ANDROID_ABI}/libavutil.so )

add_library( swresample
             SHARED
             IMPORTED )
set_target_properties( swresample
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../src/main/jniLibs/${ANDROID_ABI}/libswresample.so )

add_library( avresample
             SHARED
             IMPORTED )
set_target_properties( avresample
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../src/main/jniLibs/${ANDROID_ABI}/libavresample.so )


include_directories(src/main/jniLibs/include)

target_link_libraries( native-lib
                       avcodec avfilter swscale avformat avutil swresample avresample
                       ${log-lib} )