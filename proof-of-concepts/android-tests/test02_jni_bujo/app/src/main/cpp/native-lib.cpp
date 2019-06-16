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

    bujo::detector::FilteringOptions f_opts;
    bujo::curves::CurveGenerationOptions cg_opts;
    bujo::curves::WordDetectionOptions w_opts;
    f_opts.cutoff_quantile = 0.95f;
    w_opts.cutoff_word_std = 0.02;

    det.loadImage(srcImage, 0.2f, 0.5f, f_opts);
    det.updateRegionAuto(1.2f, 100, 10.0f, 0.0f, 0.05f);

    det.selectSupportCurvesAuto(6, 25, 0.5f, 0.5f, 0.5f, cg_opts);
    det.detectLines(25);


    det.detectWords(5, 4, 0.1f, w_opts);

    env->ReleaseIntArrayElements(pixels_, pixels, 0);

    return det.numWords();
}