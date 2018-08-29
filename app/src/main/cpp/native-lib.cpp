#include <jni.h>
#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <string>
#include <unistd.h>

#define LOG_TAG "FFNative"
#define ALOGV(...) ((void)__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__))
#define ALOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))
#define ALOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__))
#define ALOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__))
#define ALOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))

extern "C" {

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libswscale/swscale.h>
#include "libswresample/swresample.h"
#include "libavutil/opt.h"
#include <libavutil/imgutils.h>


JNIEXPORT jstring JNICALL
Java_cc_dewdrop_ffplayer_utils_FFUtils_urlProtocolInfo(JNIEnv *env, jclass type) {
    char info[40000] = {0};
    av_register_all();

    struct URLProtocol *pup = NULL;

    struct URLProtocol **p_temp = (struct URLProtocol **) &pup;
    avio_enum_protocols((void **) p_temp, 0);

    while ((*p_temp) != NULL) {
        sprintf(info, "%sInput: %s\n", info, avio_enum_protocols((void **) p_temp, 0));
    }
    pup = NULL;
    avio_enum_protocols((void **) p_temp, 1);
    while ((*p_temp) != NULL) {
        sprintf(info, "%sInput: %s\n", info, avio_enum_protocols((void **) p_temp, 1));
    }
    ALOGI("%s", info);
    return env->NewStringUTF(info);
}

JNIEXPORT jstring JNICALL
Java_cc_dewdrop_ffplayer_utils_FFUtils_avFormatInfo(JNIEnv *env, jclass type) {
    char info[40000] = {0};

    av_register_all();

    AVInputFormat *if_temp = av_iformat_next(NULL);
    AVOutputFormat *of_temp = av_oformat_next(NULL);
    while (if_temp != NULL) {
        sprintf(info, "%sInput: %s\n", info, if_temp->name);
        if_temp = if_temp->next;
    }
    while (of_temp != NULL) {
        sprintf(info, "%sOutput: %s\n", info, of_temp->name);
        of_temp = of_temp->next;
    }
    ALOGI("%s", info);
    return env->NewStringUTF(info);
}

JNIEXPORT jstring JNICALL
Java_cc_dewdrop_ffplayer_utils_FFUtils_avCodecInfo(JNIEnv *env, jclass type) {
    char info[40000] = {0};

    av_register_all();

    AVCodec *c_temp = av_codec_next(NULL);

    while (c_temp != NULL) {
        if (c_temp->decode != NULL) {
            sprintf(info, "%sdecode:", info);
        } else {
            sprintf(info, "%sencode:", info);
        }
        switch (c_temp->type) {
            case AVMEDIA_TYPE_VIDEO:
                sprintf(info, "%s(video):", info);
                break;
            case AVMEDIA_TYPE_AUDIO:
                sprintf(info, "%s(audio):", info);
                break;
            case AVMEDIA_TYPE_SUBTITLE:
                sprintf(info, "%s(subtitle):", info);
                break;
            default:
                sprintf(info, "%s(other):", info);
                break;
        }
        sprintf(info, "%s[%10s]\n", info, c_temp->name);
        c_temp = c_temp->next;
    }
    ALOGI("%s", info);
    return env->NewStringUTF(info);
}

JNIEXPORT jstring JNICALL
Java_cc_dewdrop_ffplayer_utils_FFUtils_avFilterInfo(JNIEnv *env, jclass type) {
    char info[40000] = {0};
    avfilter_register_all();

    AVFilter *f_temp = (AVFilter *) avfilter_next(NULL);
    while (f_temp != NULL) {
        sprintf(info, "%s%s\n", info, f_temp->name);
        f_temp = f_temp->next;
    }
    ALOGI("%s", info);
    return env->NewStringUTF(info);
}

JNIEXPORT void JNICALL
Java_cc_dewdrop_ffplayer_utils_FFUtils_playVideo(JNIEnv *env, jclass type, jstring videoPath_,
                                                 jobject surface) {
    const char *videoPath = env->GetStringUTFChars(videoPath_, 0);
    ALOGI("PlayVideo: %s", videoPath);

    if (videoPath == NULL) {
        ALOGE("videoPath is null");
        return;
    }

    AVFormatContext *formatContext = avformat_alloc_context();

    // open video file
    ALOGI("Open video file");
    if (avformat_open_input(&formatContext, videoPath, NULL, NULL) != 0) {
        ALOGE("Cannot open video file: %s\n", videoPath);
        return;
    }

    // Retrieve stream information
    ALOGI("Retrieve stream information");
    if (avformat_find_stream_info(formatContext, NULL) < 0) {
        ALOGE("Cannot find stream information.");
        return;
    }

    // Find the first video stream
    ALOGI("Find video stream");
    int video_stream_index = -1;
    for (int i = 0; i < formatContext->nb_streams; i++) {
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
        }
    }

    if (video_stream_index == -1) {
        ALOGE("No video stream found.");
        return; // no video stream found.
    }

    // Get a pointer to the codec context for the video stream
    ALOGI("Get a pointer to the codec context for the video stream");
    AVCodecParameters *codecParameters = formatContext->streams[video_stream_index]->codecpar;

    // Find the decoder for the video stream
    ALOGI("Find the decoder for the video stream");
    AVCodec *codec = avcodec_find_decoder(codecParameters->codec_id);
    if (codec == NULL) {
        ALOGE("Codec not found.");
        return; // Codec not found
    }

    AVCodecContext *codecContext = avcodec_alloc_context3(codec);

    if (codecContext == NULL) {
        ALOGE("CodecContext not found.");
        return; // CodecContext not found
    }

    // fill CodecContext according to CodecParameters
    if (avcodec_parameters_to_context(codecContext, codecParameters) < 0) {
        ALOGD("Fill CodecContext failed.");
        return;
    }

    // init codex context
    ALOGI("open Codec");
    if (avcodec_open2(codecContext, codec, NULL)) {
        ALOGE("Init CodecContext failed.");
        return;
    }

    AVPixelFormat dstFormat = AV_PIX_FMT_RGBA;

    // Allocate av packet
    AVPacket *packet = av_packet_alloc();
    if (packet == NULL) {
        ALOGD("Could not allocate av packet.");
        return;
    }

    // Allocate video frame
    ALOGI("Allocate video frame");
    AVFrame *frame = av_frame_alloc();
    // Allocate render frame
    ALOGI("Allocate render frame");
    AVFrame *renderFrame = av_frame_alloc();

    if (frame == NULL || renderFrame == NULL) {
        ALOGD("Could not allocate video frame.");
        return;
    }

    // Determine required buffer size and allocate buffer
    ALOGI("Determine required buffer size and allocate buffer");
    int size = av_image_get_buffer_size(dstFormat, codecContext->width, codecContext->height, 1);
    uint8_t *buffer = (uint8_t *) av_malloc(size * sizeof(uint8_t));
    av_image_fill_arrays(renderFrame->data, renderFrame->linesize, buffer, dstFormat,
                         codecContext->width, codecContext->height, 1);

    // init SwsContext
    ALOGI("init SwsContext");
    struct SwsContext *swsContext = sws_getContext(codecContext->width,
                                                   codecContext->height,
                                                   codecContext->pix_fmt,
                                                   codecContext->width,
                                                   codecContext->height,
                                                   dstFormat,
                                                   SWS_BILINEAR,
                                                   NULL,
                                                   NULL,
                                                   NULL);
    if (swsContext == NULL) {
        ALOGE("Init SwsContext failed.");
        return;
    }

    // native window
    ALOGI("native window");
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);
    ANativeWindow_Buffer windowBuffer;

    // get video width , height
    ALOGI("get video width , height");
    int videoWidth = codecContext->width;
    int videoHeight = codecContext->height;
    ALOGI("VideoSize: [%d,%d]", videoWidth, videoHeight);

    // 设置native window的buffer大小,可自动拉伸
    ALOGI("set native window");
    ANativeWindow_setBuffersGeometry(nativeWindow, videoWidth, videoHeight,
                                     WINDOW_FORMAT_RGBA_8888);


    ALOGI("read frame");
    while (av_read_frame(formatContext, packet) == 0) {
        // Is this a packet from the video stream?
        if (packet->stream_index == video_stream_index) {

            // Send origin data to decoder
            int sendPacketState = avcodec_send_packet(codecContext, packet);
            if (sendPacketState == 0) {
                ALOGD("向解码器-发送数据");

                int receiveFrameState = avcodec_receive_frame(codecContext, frame);
                if (receiveFrameState == 0) {
                    ALOGD("从解码器-接收数据");
                    // lock native window buffer
                    ANativeWindow_lock(nativeWindow, &windowBuffer, NULL);

                    // 格式转换
                    sws_scale(swsContext, (uint8_t const *const *) frame->data,
                              frame->linesize, 0, codecContext->height,
                              renderFrame->data, renderFrame->linesize);

                    // 获取stride
                    uint8_t *dst = (uint8_t *) windowBuffer.bits;
                    uint8_t *src = (renderFrame->data[0]);
                    int dstStride = windowBuffer.stride * 4;
                    int srcStride = renderFrame->linesize[0];

                    // 由于window的stride和帧的stride不同,因此需要逐行复制
                    for (int i = 0; i < videoHeight; i++) {
                        memcpy(dst + i * dstStride, src + i * srcStride, srcStride);
                    }

                    ANativeWindow_unlockAndPost(nativeWindow);
                } else if (receiveFrameState == AVERROR(EAGAIN)) {
                    ALOGD("从解码器-接收-数据失败：AVERROR(EAGAIN)");
                } else if (receiveFrameState == AVERROR_EOF) {
                    ALOGD("从解码器-接收-数据失败：AVERROR_EOF");
                } else if (receiveFrameState == AVERROR(EINVAL)) {
                    ALOGD("从解码器-接收-数据失败：AVERROR(EINVAL)");
                } else {
                    ALOGD("从解码器-接收-数据失败：未知");
                }
            } else if (sendPacketState == AVERROR(EAGAIN)) {//发送数据被拒绝，必须尝试先读取数据
                ALOGD("向解码器-发送-数据包失败：AVERROR(EAGAIN)");//解码器已经刷新数据但是没有新的数据包能发送给解码器
            } else if (sendPacketState == AVERROR_EOF) {
                ALOGD("向解码器-发送-数据失败：AVERROR_EOF");
            } else if (sendPacketState == AVERROR(EINVAL)) {//遍解码器没有打开，或者当前是编码器，也或者需要刷新数据
                ALOGD("向解码器-发送-数据失败：AVERROR(EINVAL)");
            } else if (sendPacketState == AVERROR(ENOMEM)) {//数据包无法压如解码器队列，也可能是解码器解码错误
                ALOGD("向解码器-发送-数据失败：AVERROR(ENOMEM)");
            } else {
                ALOGD("向解码器-发送-数据失败：未知");
            }

        }
        av_packet_unref(packet);
    }


    //内存释放
    ALOGI("release memory");
    ANativeWindow_release(nativeWindow);
    av_frame_free(&frame);
    av_frame_free(&renderFrame);
    av_packet_free(&packet);
    avcodec_close(codecContext);
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);
    avformat_free_context(formatContext);
    env->ReleaseStringUTFChars(videoPath_, videoPath);
}

}