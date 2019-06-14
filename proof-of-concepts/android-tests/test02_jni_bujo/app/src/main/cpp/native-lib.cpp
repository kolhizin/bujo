#include <jni.h>
#include <string>
#include "detector.h"
#include <xtensor/xtensor.hpp>

extern "C" JNIEXPORT jstring JNICALL
Java_com_kolhizin_test02_1bujo_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {

    bujo::detector::Detector det;
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_kolhizin_test02_1bujo_MainActivity_loadImageIntoDecoder(JNIEnv *env, jobject instance,
                                                                 jintArray pixels_, jint width,
                                                                 jint height) {
    jint *pixels = env->GetIntArrayElements(pixels_, NULL);

    // TODO
    bujo::detector::Detector det;

    unsigned long w = width, h = height;
    unsigned char * ptr = reinterpret_cast<unsigned char *>(pixels);
    unsigned long stride = width * 4;

    xt::xtensor<float, 2> srcImage({h, w});
    for(int i = 0; i < h; i++)
        for(int j = 0; j < w; j++)
        {
            unsigned char * p = ptr + stride * i + j * 4;
            float r = static_cast<float>(p[0]) + static_cast<float>(p[1]) + static_cast<float>(p[2]);
            srcImage.at(i, j) = r / (255.0f * 3.0f);
        }

    bujo::detector::FilteringOptions opts;
    opts.cutoff_quantile = 0.95f;
    det.loadImage(srcImage, 0.2f, 0.5f, opts);

    env->ReleaseIntArrayElements(pixels_, pixels, 0);

    return det.textDelta();
}