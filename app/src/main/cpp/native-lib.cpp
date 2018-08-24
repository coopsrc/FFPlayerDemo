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

    //注册FFmpeg所有编解码器，以及相关协议。
    av_register_all();

    //分配结构体
    AVFormatContext *formatContext = avformat_alloc_context();

    //打开视频数据源。由于Android 对SDK存储权限的原因，如果没有为当前项目赋予SDK存储权限，打开本地视频文件时会失败
    int open_state = avformat_open_input(&formatContext, videoPath, NULL, NULL);
    if (open_state < 0) {
        char errbuf[128];
        if (av_strerror(open_state, errbuf, sizeof(errbuf) == 0)) {
            ALOGE("open video file error: %s", errbuf);
        }
        return;
    }

    //为分配的AVFormatContext 结构体中填充数据
    if (avformat_find_stream_info(formatContext, NULL) < 0) {
        ALOGE("Read video stream failed.");
        return;
    }

    int video_stream_index = -1;
    ALOGD("当前视频数据，包含的数据流数量：%d", formatContext->nb_streams);
    for (int i = 0; i < formatContext->nb_streams; i++) {

        //如果是数据流的编码格式为AVMEDIA_TYPE_VIDEO——视频流。
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;//记录视频流下标
            break;
        }
    }
    if (video_stream_index == -1) {
        ALOGD("没有找到 视频流。");
        return;
    }

    //通过编解码器的id——codec_id 获取对应（视频）流解码器
    AVCodecParameters *codecParameters = formatContext->streams[video_stream_index]->codecpar;
    AVCodec *videoDecoder = avcodec_find_decoder(codecParameters->codec_id);

    if (videoDecoder == NULL) {
        ALOGD("未找到对应的流解码器。");
        return;
    }
    //通过解码器分配(并用  默认值   初始化)一个解码器context
    AVCodecContext *codecContext = avcodec_alloc_context3(videoDecoder);

    if (codecContext == NULL) {
        ALOGD("分配 解码器上下文失败。");
        return;
    }
    //更具指定的编码器值填充编码器上下文
    if (avcodec_parameters_to_context(codecContext, codecParameters) < 0) {
        ALOGD("填充编解码器上下文失败。");
        return;
    }
    //通过所给的编解码器初始化编解码器上下文
    if (avcodec_open2(codecContext, videoDecoder, NULL) < 0) {
        ALOGD("初始化 解码器上下文失败。");
        return;
    }
    AVPixelFormat dstFormat = AV_PIX_FMT_RGBA;
    //分配存储压缩数据的结构体对象AVPacket
    //如果是视频流，AVPacket会包含一帧的压缩数据。
    //但如果是音频则可能会包含多帧的压缩数据
    AVPacket *packet = av_packet_alloc();
    //分配解码后的每一数据信息的结构体（指针）
    AVFrame *frame = av_frame_alloc();
    //分配最终显示出来的目标帧信息的结构体（指针）
    AVFrame *outFrame = av_frame_alloc();
    uint8_t *out_buffer = (uint8_t *) av_malloc(
            (size_t) av_image_get_buffer_size(dstFormat, codecContext->width, codecContext->height,
                                              1));
    //更具指定的数据初始化/填充缓冲区
    av_image_fill_arrays(outFrame->data, outFrame->linesize, out_buffer, dstFormat,
                         codecContext->width, codecContext->height, 1);
    //初始化SwsContext

    SwsContext *swsContext = sws_getContext(
            codecContext->width   //原图片的宽
            , codecContext->height  //源图高
            , codecContext->pix_fmt //源图片format
            , codecContext->width  //目标图的宽
            , codecContext->height  //目标图的高
            , dstFormat, SWS_BICUBIC, NULL, NULL, NULL
    );
    if (swsContext == NULL) {
        ALOGD("swsContext==NULL");
        return;
    }
    //Android 原生绘制工具
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);
    //定义绘图缓冲区
    ANativeWindow_Buffer outBuffer;
    //通过设置宽高限制缓冲区中的像素数量，而非屏幕的物流显示尺寸。
    //如果缓冲区与物理屏幕的显示尺寸不相符，则实际显示可能会是拉伸，或者被压缩的图像
    ANativeWindow_setBuffersGeometry(nativeWindow, codecContext->width, codecContext->height,
                                     WINDOW_FORMAT_RGBA_8888);
    //循环读取数据流的下一帧
    while (av_read_frame(formatContext, packet) == 0) {

        if (packet->stream_index == video_stream_index) {
            //讲原始数据发送到解码器
            int sendPacketState = avcodec_send_packet(codecContext, packet);
            if (sendPacketState == 0) {
                int receiveFrameState = avcodec_receive_frame(codecContext, frame);
                if (receiveFrameState == 0) {
                    //锁定窗口绘图界面
                    ANativeWindow_lock(nativeWindow, &outBuffer, NULL);
                    //对输出图像进行色彩，分辨率缩放，滤波处理
                    sws_scale(swsContext, (const uint8_t *const *) frame->data, frame->linesize, 0,
                              frame->height, outFrame->data, outFrame->linesize);
                    uint8_t *dst = (uint8_t *) outBuffer.bits;
                    //解码后的像素数据首地址
                    //这里由于使用的是RGBA格式，所以解码图像数据只保存在data[0]中。但如果是YUV就会有data[0]
                    //data[1],data[2]
                    uint8_t *src = outFrame->data[0];
                    //获取一行字节数
                    int oneLineByte = outBuffer.stride * 4;
                    //复制一行内存的实际数量
                    int srcStride = outFrame->linesize[0];
                    for (int i = 0; i < codecContext->height; i++) {
                        memcpy(dst + i * oneLineByte, src + i * srcStride, srcStride);
                    }
                    //解锁
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
    ANativeWindow_release(nativeWindow);
    av_frame_free(&outFrame);
    av_frame_free(&frame);
    av_packet_free(&packet);
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);
    avformat_free_context(formatContext);
    env->ReleaseStringUTFChars(videoPath_, videoPath);
}

}