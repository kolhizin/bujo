#include <jni.h>
#include <string>
#include "AsyncDetect.h"
#include "detector.h"

extern "C"
JNIEXPORT jint JNICALL
Java_com_kolhizin_detectbujo_AsyncDetectionTask_detect(JNIEnv *env, jobject instance,
        jobject page, jobject settings) {

    try {
        TaskNotifier notifier(env, instance, "com/kolhizin/detectbujo/AsyncDetectionTask");
        BuJoPage page_(env, page);
        BuJoSettings settings_(env, settings);

        try {
            asyncDetection(page_, settings_, notifier);
        }catch (const std::exception &e)
        {
            page_.setError(e.what());
            notifier.notify();
            return 1;
        }catch (...)
        {
            page_.setError("Unknown error!");
            notifier.notify();
            return 1;
        }

    }catch (...)
    {
        return -1;
    }
    return 0;
}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_kolhizin_detectbujo_Detector_createDetector(JNIEnv *env, jobject instance) {
    return reinterpret_cast<long>(new bujo::detector::Detector());
}

extern "C"
JNIEXPORT void JNICALL
Java_com_kolhizin_detectbujo_Detector_deleteDetector(JNIEnv *env, jobject instance, jlong handle) {
    delete reinterpret_cast<bujo::detector::Detector *>(handle);
}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_kolhizin_detectbujo_Detector_createPage(JNIEnv *env, jobject instance, jobject page) {
    return reinterpret_cast<long>(new BuJoPage(env, page));
}

extern "C"
JNIEXPORT void JNICALL
Java_com_kolhizin_detectbujo_Detector_deletePage(JNIEnv *env, jobject instance, jlong handle) {
    delete reinterpret_cast<BuJoPage *>(handle);
}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_kolhizin_detectbujo_Detector_createSettings(JNIEnv *env, jobject instance,
                                                     jobject settings) {
    return reinterpret_cast<long>(new BuJoSettings(env, settings));
}

extern "C"
JNIEXPORT void JNICALL
Java_com_kolhizin_detectbujo_Detector_deleteSettings(JNIEnv *env, jobject instance, jlong handle) {
    delete reinterpret_cast<BuJoSettings *>(handle);
}


extern "C"
JNIEXPORT jstring JNICALL
Java_com_kolhizin_detectbujo_Detector_load__JJJ(JNIEnv *env, jobject instance, jlong hDetector,
                                                jlong hPage, jlong hSettings) {
    auto pDetector = reinterpret_cast<bujo::detector::Detector *>(hDetector);
    auto pPage = reinterpret_cast<BuJoPage *>(hPage);
    auto pSettings = reinterpret_cast<BuJoSettings *>(hSettings);
    try {
        runLoad(*pDetector, *pPage, *pSettings);
    }catch (const std::exception &e)
    {
        return env->NewStringUTF(e.what());
    }catch (...)
    {
        return env->NewStringUTF("Unknwon error");
    }
    return env->NewStringUTF(""); //empty means ok
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_kolhizin_detectbujo_Detector_preprocess__JJJ(JNIEnv *env, jobject instance,
                                                      jlong hDetector, jlong hPage,
                                                      jlong hSettings) {
    auto pDetector = reinterpret_cast<bujo::detector::Detector *>(hDetector);
    auto pPage = reinterpret_cast<BuJoPage *>(hPage);
    auto pSettings = reinterpret_cast<BuJoSettings *>(hSettings);
    try {
        runPreprocess(*pDetector, *pPage, *pSettings);
    }catch (const std::exception &e)
    {
        return env->NewStringUTF(e.what());
    }catch (...)
    {
        return env->NewStringUTF("Unknwon error");
    }
    return env->NewStringUTF(""); //empty means ok
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_kolhizin_detectbujo_Detector_detectRegions__JJJ(JNIEnv *env, jobject instance,
                                                         jlong hDetector, jlong hPage,
                                                         jlong hSettings) {
    auto pDetector = reinterpret_cast<bujo::detector::Detector *>(hDetector);
    auto pPage = reinterpret_cast<BuJoPage *>(hPage);
    auto pSettings = reinterpret_cast<BuJoSettings *>(hSettings);
    try {
        runDetectRegions(*pDetector, *pPage, *pSettings);
    }catch (const std::exception &e)
    {
        return env->NewStringUTF(e.what());
    }catch (...)
    {
        return env->NewStringUTF("Unknwon error");
    }
    return env->NewStringUTF(""); //empty means ok
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_kolhizin_detectbujo_Detector_detectLines__JJJ(JNIEnv *env, jobject instance,
                                                       jlong hDetector, jlong hPage,
                                                       jlong hSettings) {
    auto pDetector = reinterpret_cast<bujo::detector::Detector *>(hDetector);
    auto pPage = reinterpret_cast<BuJoPage *>(hPage);
    auto pSettings = reinterpret_cast<BuJoSettings *>(hSettings);
    try {
        runDetectLines(*pDetector, *pPage, *pSettings);
    }catch (const std::exception &e)
    {
        return env->NewStringUTF(e.what());
    }catch (...)
    {
        return env->NewStringUTF("Unknwon error");
    }
    return env->NewStringUTF(""); //empty means ok
}